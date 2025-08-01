#pragma once

#include <memory>
#include <list>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <condition_variable>

#include "coroutine.h"
#include "macro.h"
#include "thread.h"

namespace agent
{
    class Scheduler
    {
    public:
        using ptr = std::shared_ptr<Scheduler>;
        typedef Mutex MutexType;

        // Scheduler();
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "UNKNOW", bool idleFlag = true);
        virtual ~Scheduler();

        const std::string getName() const {return m_name;}

        static Scheduler* GetThis();
        static Coroutine* GetMainCoroutine();

        void start();
        void stop();

        template<typename CorOrCb>
        void schedule(CorOrCb cc, int thread = -1)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = scheduleNoLock(cc, thread);
            }
            if(need_tickle)
            {
                m_con.notify_one();
            }
        }
        template<typename InputIterator>
        void schedule(InputIterator begin, InputIterator end)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while(begin != end)
                {
                    need_tickle = scheduleNoLock(&*begin) || need_tickle;
                }
            }
            if(need_tickle)
            {
                m_con.notify_one();
            }
        }
    private:
        template<typename CorOrCb>
        bool scheduleNoLock(CorOrCb cc, int thread)
        {
            bool need_tickle = m_coroutines.empty();
            CoroutineAndThread ct(cc, thread);
            if(ct.coroutine || ct.cb)
            {
                m_coroutines.push_back(ct);
            }
            return need_tickle;
        }

    protected:
        virtual void tickle();
        void run();
        virtual bool stopping();
        virtual void idle();

        void setThis();

    private:
        typedef struct CoroutineAndThread
        {
            Coroutine::ptr coroutine;
            std::function<void()> cb;
            int threadId;

            CoroutineAndThread(Coroutine::ptr c, int thr)
            :coroutine(c),threadId(thr)
            {}

            CoroutineAndThread(Coroutine::ptr * f, int thr)
            :threadId(thr)
            {
                coroutine.swap(*f);
            }
            
            CoroutineAndThread(std::function<void()> f, int thr)
            :cb(f), threadId(thr)
            {

            }

            CoroutineAndThread(std::function<void()>* f, int thr)
            :threadId(thr)
            {
                cb.swap(*f);
            }

            CoroutineAndThread()
            :threadId(-1)
            {}

            void reset()
            {
                coroutine = nullptr;
                cb = nullptr;
                threadId = -1;
            }
        }CoroutineAndThread;

    private:
        MutexType m_mutex;
        std::mutex m_uni_mutex;
        std::condition_variable m_con;
        std::vector<Thread::ptr> m_threads;
        std::list<CoroutineAndThread> m_coroutines;
        Coroutine::ptr m_mainCoroutine;

    
    protected:
        std::vector<int> m_threadIds;                       // 所有的线程id
        size_t m_threadCount = 0;                           // 线程总数 
        std::atomic<size_t> m_activeCoroutineCount = {0};        // 活跃的线程数
        bool m_stopping = false;                             // 线程状态
        int m_mainThreadId = 0;                             // 主线程id
        std::string m_name;
        std::atomic<bool> m_idleFlag {true};
    };
}