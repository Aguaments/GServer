#include "coroutine.h"
#include <atomic>

// #include <sys/types.h>

#include "macro.h"
#include "config.h"


namespace agent{
    static std::atomic<uint64_t> s_coroutine_id {0};
    static std::atomic<uint64_t> s_coroutine_count {0};

    static thread_local Coroutine* t_coroutine = nullptr; // 获取当前协程、主协程 方式
    static thread_local std::shared_ptr<Coroutine::ptr> t_thread_coroutine = nullptr;

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
    {
        m_state = State::EXEC;
        SetThis(this);

        if(getcontext(&m_ctx))
        {
            AGENT_ASSERT_PARA(false, "getcontext error");
        }

        ++s_coroutine_count;
    }

    Coroutine::Coroutine(std::function<void()> cb, size_t stacksize = 0)
    :m_id(++s_coroutine_id), m_cb(cb)
    {
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
            AGENT_ASSERT(m_state == State::TERM || m_state == State::INIT);
            StackMemoryPool::Free(m_stack, m_stacksize);
        }
        else{
            AGENT_ASSERT(!m_cb);
            AGENT_ASSERT(m_state == State::EXEC);

            Coroutine* cur = t_coroutine;
            if(cur == this)
            {
                SetThis(nullptr);
            }
        }
    }

    // 重置协程函数，在init中或者term状态
    void reset(std::function<void()> cb);
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();
    // 返回当前协程
    static Coroutine::ptr GetThis();
    
    // 协程切换到后台，设置ready状态
    static void YieldToReady();
    // 协程切换到后台，设置hold状态
    static void YieldToHold();

    static uint64_t TotalCoroutine();

    static void MainFunc();
}