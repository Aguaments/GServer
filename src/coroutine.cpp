#include "coroutine.h"
#include <atomic>

// #include <sys/types.h>

#include "macro.h"
#include "config.h"
#include "log.h"
#include "scheduler.h"


namespace agent{
    static Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    static std::atomic<uint64_t> s_coroutine_id {0};
    static std::atomic<uint64_t> s_coroutine_count {0};

    static thread_local Coroutine* t_coroutine = nullptr; // 获取当前协程、主协程 方式
    static thread_local Coroutine::ptr t_thread_coroutine = nullptr; // 主协程

    static ConfigVar<uint32_t>::ptr g_coroutine_stack_size = 
        Config::Lookup<uint32_t>("coroutine.stack_size", 1024 * 1024, "fiber stack size");

    class MemoryPool
    {
    public:
        static void* Alloc(size_t size)
        {
            return malloc(size);
        }

        static void Free(void* vp ,size_t size)
        {
            return free(vp);
        }
    };

    using StackMemoryPool = MemoryPool;

    Coroutine::Coroutine()
    :m_id(s_coroutine_id ++)
    {
        // 创建主协程不需要栈空间， 同时主协程创建后即为运行态，不需要swapin切换执行
        m_state = State::EXEC;
        m_name = "Main Coroutine";
        SetThis(this);

        if(getcontext(&m_ctx))
        {
            AGENT_ASSERT_PARA(false, "getcontext error");
        }

        ++s_coroutine_count;
        AGENT_LOG_INFO(g_logger) << "[Init Main coroutine]: " << m_id << " name: "<< m_name;
    }

    Coroutine::Coroutine(std::function<void()> cb, size_t stacksize, bool use_caller, std::string name)
    :m_id(s_coroutine_id++), m_cb(cb), m_name(name)
    {
        AGENT_LOG_INFO(g_logger) << "[Init coroutine] Courtine_ID: [" << m_id << "] Coroutine_Name: ["<< m_name << "]";
        ++s_coroutine_count;
        m_stacksize = stacksize ? stacksize : g_coroutine_stack_size -> getValue();

        m_stack = StackMemoryPool::Alloc(m_stacksize);
        if(getcontext(&m_ctx))
        {
            AGENT_ASSERT_PARA(false, "getcontext error");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &MainFunc, 0);
    }

    Coroutine::~Coroutine()
    {
        --s_coroutine_count;
        if(m_stack)
        {
            AGENT_ASSERT(m_state == State::TERM || m_state == State::INIT || m_state == State::EXCEPT);
            StackMemoryPool::Free(m_stack, m_stacksize);
            AGENT_LOG_DEBUG(g_logger) << "[End coroutine]: " << m_id;
        }
        else{
            // 主协程没有分配独立的栈，使用的是线程的栈，因此会走到无栈的这条路径上进行协程的释放
            AGENT_ASSERT(!m_cb);
            AGENT_ASSERT(m_state == State::EXEC);

            Coroutine* cur = t_coroutine;
            if(cur == this)
            {
                SetThis(nullptr);
            }
            AGENT_LOG_DEBUG(g_logger) << "[End Main coroutine]: " << m_id;
        }
    }

    // 重置协程函数，协程执行完毕后内存没有释放，使用该函数可以重置为新的协程继续运行
    void Coroutine::reset(std::function<void()> cb, std::string name)
    {
        AGENT_ASSERT(m_stacksize);
        AGENT_ASSERT(m_state == State::TERM 
                || m_state == State::INIT
                || m_state == State::EXCEPT)
        m_cb = cb;
        m_name = name;
        if(getcontext(&m_ctx))
        {
            AGENT_ASSERT_PARA(false, "getcontext");
        }
        m_ctx.uc_link = nullptr;
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = m_stacksize;

        makecontext(&m_ctx, &MainFunc, 0);
        m_state = State::INIT;
    }
    // 切换到当前协程执行，将正在运行的协程切换到后台，调用的协程转为运行。主协程切换当前协程
    void Coroutine::swapIn()
    {   
        //AGENT_LOG_DEBUG(g_logger) << "[Start swapin]: Coroutine name = " << this -> m_name;
        SetThis(this);
        // AGENT_LOG_DEBUG(g_logger) << int(m_state) << " name : " << this -> getName();
        AGENT_ASSERT(m_state != State::EXEC);
        m_state = State::EXEC;

        if(swapcontext(&(Scheduler::GetMainCoroutine() -> m_ctx), &m_ctx))
        {
            AGENT_ASSERT_PARA(false, "swapcontext");
        }
    }
    // 切换到后台执行，当前协程切换到主协程
    void Coroutine::swapOut()
    {
        //AGENT_LOG_DEBUG(g_logger) << "[Start swapout]: Current coroutine name = " << this -> m_name;
        SetThis(Scheduler::GetMainCoroutine());
        if(swapcontext(&m_ctx, &(Scheduler::GetMainCoroutine() -> m_ctx)))
        {
            AGENT_ASSERT_PARA(false, "swapcontext");
        }
    }

    void Coroutine::call()
    {
        SetThis(this);
        m_state = State::EXEC;
        if(swapcontext(&(t_thread_coroutine -> m_ctx), &m_ctx)) // 激活协程中的函数调用
        {
            AGENT_ASSERT_PARA(false, "swapcontext error");
        }
    }

    void Coroutine::back()
    {
        SetThis(t_thread_coroutine.get());
        if(swapcontext(&m_ctx, &(t_thread_coroutine -> m_ctx)))
        {
            AGENT_ASSERT_PARA(false, "swapcontext");
        }
    }

    void Coroutine::SetThis(Coroutine* cor)
    {
        t_coroutine = cor;
    }

    // 返回当前协程，创建的时候就是用getthis
    Coroutine::ptr Coroutine::GetThis()
    {
        if(t_coroutine)
        {
            return t_coroutine -> shared_from_this();
        }
        Coroutine::ptr main_coroutine(new Coroutine());
        AGENT_ASSERT(t_coroutine == main_coroutine.get());
        t_thread_coroutine = main_coroutine;
        return t_coroutine -> shared_from_this();
    }
    
    // 协程切换到后台，设置ready状态
    void Coroutine::YieldToReady()
    {
        Coroutine::ptr cur = GetThis();
        cur -> m_state = State::READY;
        cur -> swapOut(); // 当前协程切换回主协程
    }
    // 协程切换到后台，设置hold状态
    void Coroutine::YieldToHold()
    {
        Coroutine::ptr cur = GetThis();
        cur -> m_state = State::HOLD;
        cur -> swapOut();
    }

    uint64_t Coroutine::TotalCoroutine()
    {
        return s_coroutine_count;
    }

    void Coroutine::MainFunc()
    {
        Coroutine::ptr cur = GetThis(); // 在协程栈上创建的智能指针
        AGENT_LOG_INFO(g_logger) << "[Start Main Func] Coroutine name: " << cur -> getName() << " Coroutine num: " << cur -> GetCoroutineId();
        AGENT_ASSERT(cur);
        try
        {
            cur -> m_cb();
            cur -> m_cb = nullptr;
            cur -> m_state = State::TERM;
        }
        catch(std::exception& e)
        {
            cur -> m_state = State::EXCEPT;
            AGENT_LOG_ERROR(g_logger) << "Coroutine Except: " << e.what();
        }
        catch(...){
            cur -> m_state = State::EXCEPT;
            AGENT_LOG_ERROR(g_logger) << "Coroutine Except";
        }

        
        auto raw_ptr = cur.get();
        AGENT_LOG_INFO(g_logger) << "[End Main Func] Coroutine name: " << raw_ptr -> getName() << " Coroutine num: " << raw_ptr ->  getId();
        cur.reset();
        if(raw_ptr == Scheduler::GetMainCoroutine())
        {
            raw_ptr -> back();
        }
        else{
            raw_ptr -> swapOut();
        }
        std::cout << raw_ptr << ": " << Scheduler::GetMainCoroutine();
        AGENT_LOG_DEBUG(g_logger) << "never reach";
    }

    void Coroutine::CallerMainFunc()
    {
        Coroutine::ptr cur = GetThis(); // 在协程栈上创建的智能指针
        AGENT_ASSERT(cur);
        try
        {
            cur -> m_cb();
            cur -> m_cb = nullptr;
            cur -> m_state = State::TERM;
        }
        catch(std::exception& e)
        {
            cur -> m_state = State::EXCEPT;
            AGENT_LOG_ERROR(g_logger) << "Coroutine Except: " << e.what();
        }
        catch(...){
            cur -> m_state = State::EXCEPT;
            AGENT_LOG_ERROR(g_logger) << "Coroutine Except";
        }

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr -> back();
    }

    uint64_t Coroutine::GetCoroutineId()
    {
        if(t_coroutine)
        {
            return t_coroutine -> getId();
        }
        return 0;
    }
}