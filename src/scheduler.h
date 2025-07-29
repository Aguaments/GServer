#pragma once

#include <memory>
#include <list>
#include <vector>
#include <string>
#include <functional>
#include <atomic>

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
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "UNKNOW");
        virtual ~Scheduler();

        const std::string getName() const {return m_name;}

        static Scheduler* GetThis();
        static Coroutine* GetMainCoroutine();

        void start();
        void stop();

        template<typename CorOrCb>
        void schedule(CorOrCb cc, int thread = -1)
        {
            MutexType::Lock lock(m_mutex);
            bool need_tickle = scheduleNoLock(cc, thread);
            if(need_tickle)
            {
                tickle();
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
                tickle();
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
        std::vector<Thread::ptr> m_threads;
        std::list<CoroutineAndThread> m_coroutines;
        Coroutine::ptr m_mainCoroutine;
        std::string m_name;

    
    protected:
        std::vector<int> m_threadIds;                       // 所有的线程id
        size_t m_threadCount = 0;                           // 线程总数 
        std::atomic<size_t> m_activeThreadCount = {0};        // 活跃的线程数
        std::atomic<size_t> m_idleThreadCount = {0};          // 空闲线程数量
        bool m_stopping = true;                             // 线程状态
        bool m_normalStop = false;                          // 是否为主动停止
        int m_mainThreadId = 0;                             // 主线程id

    };
}