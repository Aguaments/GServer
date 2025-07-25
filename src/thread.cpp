#include "thread.h"
#include "log.h"

namespace agent{
    static thread_local Thread* t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";

    static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    Semaphore::Semaphore(uint32_t count)
    {
        if(sem_init(&m_semaphore, 0, count))
        {
            throw std::logic_error("sem_init error");
        }
    }
    
    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }

    void Semaphore::wait()
    {
        if(sem_wait(&m_semaphore))
        {
            throw std::logic_error("sem_wait error");
        }
    }
    
    void Semaphore::notify()
    {
        if(sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post error");
        }
    }

    Thread* Thread::GetThis()
    {
        return t_thread;
    }

    const std::string& Thread::GetName()
    {
        return t_thread_name;
    }
    void Thread::SetName(const std::string& name)
    {
        if(t_thread)
        {
            t_thread -> m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(std::function<void()> cb, const std::string& name)
    :m_cb(cb)
    ,m_name(name)
    {
        if(m_name.empty())
        {
            m_name = "UNKNOW";
        }
        int rt = pthread_create(
            &m_thread, 
            nullptr, 
            &run, 
            this);
        if(rt)
        {
            AGENT_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait(); // 如果pthread_create中的run函数没有执行完毕，会阻塞主线程，如果在run中的函数中没有执行完，该线程的创建就会变成串行执行。
    }

    Thread::~Thread()
    {
        if(m_thread)
        {
            pthread_detach(m_thread);
        }
    }

    // 
    void* Thread::run(void * arg)
    {
        Thread* thread = (Thread*)arg;
        t_thread = thread;
        t_thread_name= thread->m_name;
        thread -> m_id = agent::Utils::getThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        std::function<void()> cb;
        cb.swap(thread ->m_cb);
        thread -> m_semaphore.notify();
        cb();
        return 0;
    }

    void Thread::join()
    {
        if(m_thread)
        {
            int rt = pthread_join(m_thread, nullptr);
            if(rt)
            {
                AGENT_LOG_ERROR(g_logger) << "pthread_join fail, rt=" << rt << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    } 
}