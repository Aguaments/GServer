#pragma once
// 使用协程实现高并发
// 写少读多，需要实现读写锁

#include <thread>
#include <functional>
#include <memory>
#include <string>

#include <sys/types.h> 
#include <pthread.h>
#include <semaphore.h>

namespace agent{

    class Semaphore
    {
    public:
        Semaphore(uint32_t count=0);
        ~Semaphore();

        void wait();
        void notify();
    private:
        Semaphore(const Semaphore&) = delete;
        Semaphore(const Semaphore&&) = delete;
        Semaphore& operator=(const Semaphore&) = delete;
    
    private:
        sem_t m_semaphore;
    };

    template<class T>
    struct ScopedLockImpl
    {
    public:
        ScopedLockImpl(T& mutex)
        :m_mutex(mutex)
        {
            m_mutex.lock();
            m_locked = true;
        }
        ~ScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if(!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if(m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked = false;
    };

    template<class T>
    struct ReadScopedLockImpl
    {
    public:
        ReadScopedLockImpl(T& mutex)
        :m_mutex(mutex)
        {
            m_mutex.rdlock();
            m_locked = true;
        }
        ~ReadScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if(!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if(m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked;
    };

    
    template<class T>
    struct WriteScopedLockImpl
    {
    public:
        WriteScopedLockImpl(T& mutex)
        :m_mutex(mutex)
        {
            m_mutex.wrlock();
            m_locked = true;
        }
        ~WriteScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if(!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if(m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked;
    };

    class RWMutex
    {
    public:
        using ReadLock = ReadScopedLockImpl<RWMutex>;
        using WriteLock = WriteScopedLockImpl<RWMutex>;
        
        RWMutex()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }
        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }
    private:
        pthread_rwlock_t m_lock;
    };

    class NullRWMutex
    {
    public:
        using ReadLock = ReadScopedLockImpl<RWMutex>;
        using WriteLock = WriteScopedLockImpl<RWMutex>;

        NullRWMutex(){}
        ~NullRWMutex(){}
        void lock(){}
        void unlock(){}
    };

    class Mutex
    {
    public:
        using Lock = ScopedLockImpl<Mutex>;

        Mutex()
        {
            pthread_mutex_init(&m_mutex,nullptr);
        }
        ~Mutex()
        {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&m_mutex);
        }
        void unlock()
        {
            pthread_mutex_unlock(&m_mutex);
        }
    private:
        pthread_mutex_t m_mutex;
    };

    class NullMutex
    {
    public:
        using Lock = ScopedLockImpl<NullMutex>;
        NullMutex(){}
        ~NullMutex(){}
        void lock(){}
        void unlock(){}
    };

    class Spinlock
    {
    public:
        using Lock = ScopedLockImpl<Spinlock>;

        Spinlock()
        {
            pthread_spin_init(&m_spinlock, 0);
        }

        ~Spinlock()
        {
            pthread_spin_destroy(&m_spinlock);
        }

        void lock()
        {
            pthread_spin_lock(&m_spinlock);
        }

        void unlock()
        {
            pthread_spin_unlock(&m_spinlock);
        }

    private:
        pthread_spinlock_t m_spinlock;
    };

    class Thread
    {
    public:
        using ptr = std::shared_ptr<Thread>;
        Thread(std::function<void()> cb, const std::string& name);
        ~Thread();

        pid_t getId() const {return m_id;}
        const std::string& getName() const {return m_name;}

        void join();
        static Thread* GetThis();
        static const std::string& GetName();
        static void SetName(const std::string& name);

        static void* run(void* arg);

    public:
        Thread(const Thread&) = delete; // 拷贝构造函数
        Thread(const Thread&&) = delete; // 移动构造函数
        Thread& operator=(const Thread) = delete;
    private:
        pid_t m_id = -1;
        pthread_t m_thread = 0;
        std::function<void()> m_cb;
        std::string m_name;

        Semaphore m_semaphore;
    };
}