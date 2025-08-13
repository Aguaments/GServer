#include <unistd.h>
#include <iostream>

#include "scheduler.h"
#include "utils.h"
#include "log.h"
#include "hook.h"

namespace agent
{
    static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    static thread_local Scheduler* t_scheduler = nullptr;
    static thread_local Coroutine* t_scheduler_coroutine = nullptr;
    // static thread_local Coroutine::ptr t_root_coroutine = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name, bool idleFlag)
    :m_name(name)
    {
        m_idleFlag.store(idleFlag, std::memory_order_release);
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
        while(!m_stopping)
        {
            AGENT_LOG_INFO(g_logger) << "[Stop] "<< "{Active Coroutine counts: " << m_activeCoroutineCount
                                 << "} {Task Number: " << m_coroutines.size() << "}";
            sleep(10);
        }
        
        if(GetThis() == this) t_scheduler = nullptr;
    }

    void Scheduler::start()
    {
        {
            MutexType::Lock lock(m_mutex);
            if(m_stopping)
            {
                return;
            }
            m_stopping = false;

            AGENT_ASSERT(m_threads.empty());

            m_threads.resize(m_threadCount);

            for(size_t i = 0; i < m_threadCount; ++ i)
            {
                m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
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
        {
            MutexType::Lock lock(m_mutex);
            m_stopping = true;
        }
        
        AGENT_LOG_INFO(g_logger) << "[Stop scheduler] wait to stop...";
        AGENT_ASSERT(m_stopping);

        m_con.notify_all();

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
        AGENT_LOG_INFO(g_logger) << "[Run] Scheduler Main loop";
        set_hook_enable(true);
        setThis();
        if(Utils::getThreadId() != m_mainThreadId)
        {
            t_scheduler_coroutine = Coroutine::GetThis().get();
        }

        
        Coroutine::ptr cb_coroutine;
        Coroutine::ptr idle_coroutine(new Coroutine([this](){this -> idle();}, 0, false, "idle coroutine"));

        CoroutineAndThread ct;
        while(!stopping())
        {
            ct.reset();
            {
                std::unique_lock<std::mutex> lk(m_uni_mutex);
                m_con.wait(lk, [this](){
                    //AGENT_LOG_INFO(g_logger) << "[Thread " << Utils::getThreadId() <<"] Wait coroutine task ...";
                    return (!m_coroutines.empty() ||  m_idleFlag) || m_stopping;
                });
                {
                    auto it = m_coroutines.begin();
                    while(it != m_coroutines.end())
                    {
                        if(it -> threadId != -1 && it -> threadId != Utils::getThreadId())
                        {
                            ++ it;
                            continue;
                        }
                        AGENT_ASSERT(it -> coroutine || it -> cb);
                        if(it -> coroutine && it -> coroutine -> getState() == Coroutine::State::EXEC)
                        {
                            ++it;
                            continue;
                        }

                        ct = *it;
                        m_activeCoroutineCount ++;
                        m_coroutines.erase(it);
                        break;
                    }
                }
            }

            if(ct.coroutine && (
                    ct.coroutine -> getState() != Coroutine::State::TERM 
                    && ct.coroutine -> getState() != Coroutine::State::EXCEPT)
            )
            {
                // AGENT_LOG_DEBUG(g_logger) << "Coroutine " << ct.coroutine -> getName();
                ct.coroutine -> swapIn();
                --m_activeCoroutineCount;

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
                std::string name = "User Coroutine";
                if(cb_coroutine){
                    cb_coroutine -> reset(ct.cb, name);
                }
                else
                {
                    cb_coroutine.reset(new Coroutine(ct.cb));
                }
                ct.reset();
                cb_coroutine -> swapIn();
                --m_activeCoroutineCount;
                if(cb_coroutine -> getState() == Coroutine::State::READY)
                {
                    schedule(cb_coroutine);
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
                cb_coroutine.reset();
            }
            else
            {
                // AGENT_LOG_DEBUG(g_logger) << "Idle Coroutine";
                if(idle_coroutine -> getState() == Coroutine::State::TERM)
                {
                    // idle_coroutine -> reset([this](){this -> idle();}, "Idle Coroutine");
                    break;
                }
                ++ m_activeCoroutineCount;
                idle_coroutine -> swapIn();
                --m_activeCoroutineCount;
                if(stopping()) break;
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
        return m_stopping && m_coroutines.empty();
    }
    void Scheduler::idle()
    {
        AGENT_LOG_INFO(g_logger) << "[Idle Coroutine] idle coroutine running" ;
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