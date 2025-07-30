#include "scheduler.h"
#include "utils.h"
#include "log.h"

namespace agent
{
    static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Coroutine* t_scheduler_coroutine = nullptr;
    // static thread_local Coroutine::ptr t_root_coroutine = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name)
    {
        AGENT_ASSERT(threads > 0);

        if(use_caller)
        {
            Coroutine::GetThis(); // 创建主协程，用于切换到run协程，已经默认有了一个协程了
            -- threads;

            AGENT_ASSERT(GetThis() == nullptr);
            t_scheduler = this;
            
            m_mainCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this), 0, true, "Scheduler coroutine"));
            
            Thread::SetName(m_name);

            t_scheduler_coroutine = m_mainCoroutine.get();
            m_mainThreadId = Utils::getThreadId();
            m_threadIds.push_back(m_mainThreadId);
        }
        else
        {
            m_mainThreadId = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        AGENT_ASSERT(m_stopping);
        if(GetThis() == this) t_scheduler = nullptr;
    }

    void Scheduler::start()
    {
        {
            MutexType::Lock lock(m_mutex);
            if(!m_stopping)
            {
                return;
            }
            m_stopping = false;

            AGENT_ASSERT(m_threads.empty());

            m_threadIds.resize(m_threadCount);
            for(size_t i = 0; i < m_threadCount; ++ i)
            {
                m_threads.push_back(Thread::ptr(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i))));
                m_threadIds.push_back(m_threads[i] -> getId()); // thread中的信号量就保证了线程初始化完毕后
            }
        }
        if(m_mainCoroutine)
        {
            m_mainCoroutine -> call();
        }
    }

    void Scheduler::stop()
    {
        m_normalStop = true;
        if(m_mainCoroutine 
            && m_threadCount == 0 
            && (m_mainCoroutine -> getState() == Coroutine::State::TERM
            || m_mainCoroutine -> getState() == Coroutine::State::INIT))
        {
            AGENT_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true; 
            if(stopping())
            {
                return;
            }
        }
        //bool exit_on_this_coroutine = false;
        if(m_mainThreadId != -1)
        {
            AGENT_ASSERT(GetThis() == this);
        }   
        else
        {
            AGENT_ASSERT(GetThis() != this);
        }

        m_stopping = true;
        for(size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }

        if(m_mainCoroutine)
        {
            tickle();
        
        }
        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for(auto& i : thrs) {
            i-> join();
        }
    }

    void Scheduler::run()
    {
        AGENT_LOG_INFO(g_logger) << "run";
        setThis();
        AGENT_LOG_DEBUG(g_logger) << "[Current Thread id] " << Utils::getThreadId();
        if(Utils::getThreadId() != m_mainThreadId)
        {
            t_scheduler_coroutine = Coroutine::GetThis().get();
        }

        
        Coroutine::ptr cb_coroutine;
        Coroutine::ptr idle_coroutine(new Coroutine(std::bind(&Scheduler::idle, this), 0, true, "idle coroutine"));

        CoroutineAndThread ct;
        while(true)
        {
            ct.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_coroutines.begin();
                while(it != m_coroutines.end())
                {
                    if(it -> threadId != -1 && it -> threadId != Utils::getThreadId())
                    {
                        ++ it;
                        tickle_me = true;
                        continue;
                    }

                    AGENT_ASSERT(it -> coroutine || it -> cb);

                    if(it -> coroutine && it -> coroutine -> getState() == Coroutine::State::EXEC)
                    {
                        ++it;
                        continue;
                    }

                    ct = *it;
                    m_activeThreadCount ++;
                    m_coroutines.erase(it);
                    break;
                }
            }

            if(tickle_me)
            {
                AGENT_LOG_INFO(g_logger) << "start tickle";
                tickle();
            }

            if(ct.coroutine && (ct.coroutine -> getState() != Coroutine::State::TERM
                            &&ct.coroutine -> getState() != Coroutine::State::EXCEPT))
            {
                ct.coroutine -> swapIn();
                --m_activeThreadCount;

                if(ct.coroutine -> getState() == Coroutine::State::READY)
                {
                    schedule(ct.coroutine);
                }
                else if(ct.coroutine -> getState() != Coroutine::State::TERM
                            && ct.coroutine -> getState() != Coroutine::State::EXCEPT)
                {
                    ct.coroutine -> m_state = Coroutine::State::HOLD;
                }
                ct.reset();
            }
            else if(ct.cb)
            {
                if(cb_coroutine){
                    cb_coroutine -> reset(ct.cb);
                }
                else
                {
                    cb_coroutine.reset(new Coroutine(ct.cb));
                }
                ct.reset();
                cb_coroutine -> swapIn();
                m_activeThreadCount --;
                if(cb_coroutine -> getState() == Coroutine::State::READY)
                {
                    schedule(cb_coroutine);
                    cb_coroutine.reset();
                }
                else if(cb_coroutine -> getState() == Coroutine::State::EXCEPT
                                    || cb_coroutine -> getState() == Coroutine::State::TERM)
                {
                    cb_coroutine -> reset(nullptr);
                }
                else
                {
                    cb_coroutine -> m_state = Coroutine::State::HOLD;
                    cb_coroutine.reset();
                }
            }
            else
            {
                if(idle_coroutine -> getState() == Coroutine::State::TERM)
                {
                    AGENT_LOG_INFO(g_logger) << "[Idle Coroutine] idle coroutine term";
                    break;
                }
                idle_coroutine -> swapIn();
                m_activeThreadCount--;
                if(idle_coroutine -> getState() != Coroutine::State::TERM 
                    && idle_coroutine -> getState() != Coroutine::State::EXCEPT)
                {
                    idle_coroutine -> m_state = Coroutine::State::HOLD;
                }
            }
        }
    }


    void Scheduler::tickle()
    {
        AGENT_LOG_INFO(g_logger) << "tickle";
    }
    bool Scheduler::stopping()
    {
        MutexType::Lock lock(m_mutex);
        return m_normalStop && m_stopping && m_coroutines.empty() && m_activeThreadCount == 0;
    }
    void Scheduler::idle()
    {
        AGENT_LOG_INFO(g_logger) << "idle" ;
    }

    Scheduler* Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Coroutine* Scheduler::GetMainCoroutine()
    {
        return t_scheduler_coroutine;
    }


    void Scheduler::setThis()
    {
        t_scheduler = this;
    }
}