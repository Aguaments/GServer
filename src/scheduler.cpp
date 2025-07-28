#include "scheduler.h"
#include "utils.h"
#include "log.h"

namespace agent
{
    static agent::Logger::ptr g_logger = AGENT_LOG_NAME("system");

    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Coroutine* t_coroutine = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name)
    {
        AGENT_ASSERT(threads > 0);

        if(use_caller)
        {
            agent::Coroutine::GetThis();
            -- threads;

            AGENT_ASSERT(GetThis() == nullptr);
            t_scheduler = this;

            m_mainCoroutine.reset(new Coroutine(std::bind(&Scheduler::run, this)));
            Thread::SetName(m_name);

            t_coroutine = m_mainCoroutine.get();
            m_mainThreadId = Utils::getThreadId();
            m_threadIds.push_back(m_mainThreadId);
        }
        else
        {
            m_mainThreadId = -1;
        }

        m_threadCount = threads;
    }

    Scheduler::Scheduler()
    {
        AGENT_ASSERT(m_stopping);
        if(GetThis() == this) t_scheduler = nullptr;
    }

    Scheduler* Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Coroutine* Scheduler::GetMainCouroutine()
    {
        return t_coroutine;
    }

    void Scheduler::start()
    {
        MutexType::Lock lock(m_mutex);
        if(!m_stopping)
        {
            return;
        }
        m_stopping = false;

        AGENT_ASSERT(m_threadIds.empty());

        m_threadIds.resize(m_threadCount);
        for(size_t i = 0; i < m_threadCount; ++ i)
        {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i] -> getId()); // thread中的信号量就保证了线程初始化完毕后
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
        bool exit_on_this_coroutine = false;
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

        // if(exit_on_this_coroutine)
        // {

        // }

        if(m_mainCoroutine)
        {
            tickle();
        }
        if(stopping())
        {
            return;
        }
    }

    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    void Scheduler::run()
    {
        setThis();
        if(Utils::getThreadId() != m_mainThreadId)
        {
            t_coroutine = Coroutine::GetThis().get();
        }

        Coroutine::ptr idle_coroutine(new Coroutine(std::bind(idle, this)));
        Coroutine::ptr cb_coroutine;

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
                    tickle_me = true;
                    m_coroutines.erase(it);
                }
            }

            if(tickle_me)
            {
                tickle();
            }

            if(ct.coroutine && ct.coroutine -> getState() != Coroutine::State::TERM)
            {
                ct.coroutine -> swapIn();

                if(ct.coroutine -> getState() == Coroutine::State::READY)
                {
                    schedule(ct.coroutine);
                }
                else if(ct.coroutine -> getState() != Coroutine::State::TERM
                            && ct.coroutine -> getState() != Coroutine::State::EXCEPT)
                {
                    ct.coroutine -> setState(Coroutine::State::HOLD);
                }
                ct.reset();
            }
            else if(ct.cb)
            {
                
            }
        }
    }

    
}