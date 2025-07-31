#pragma once

#include <vector>
#include <atomic>
#include <memory>
#include <functional>

#include "scheduler.h"
#include "thread.h"

namespace agent
{
    class IOManager: public Scheduler
    {
    public:
        using ptr = std::shared_ptr<IOManager>;
        using RWMutexType = RWMutex;

        enum EventType
        {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0X2
        };
    public:
        IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
        ~IOManager();


        int addEvent(int fd, EventType event, std::function<void()> cb = nullptr);
        bool delEvent(int fd, EventType event);
        bool cancelEvent(int fd, EventType event);
        bool cancelAll(int fd);  // 取消某句柄下的所有事件

        static IOManager* GetThis();  // 获取当前的io manager
    
    protected:
        void tickle() override;
        bool stopping() override;
        void idle() override;

        void contextResize(size_t size);

    private:
        typedef struct{
            typedef Mutex MutexType;
            typedef struct{
                Scheduler* scheduler = nullptr;       // 执行事件的scheduler
                Coroutine::ptr coroutine;   // 事件的协程
                std::function<void()> cb;   // 事件的回调
            }EventContext;

            EventContext& getContext(EventType event);
            void resetContext(EventContext& ctx);
            void triggerEvent(EventType ev);

            int fd;
            EventContext read;      // 读事件
            EventContext write;     // 写事件
            EventType event = NONE;   // 已注册的事件
            MutexType mutex;
        }FdContext;

        int m_epfd = 0;
        int m_eventfd = 0;
        int m_tickleFds[2];

        std::atomic<size_t> m_pendingEventCount = 0;
        RWMutexType m_mutex;
        std::vector<FdContext*> m_fdContexts; 
    };

} // namespace agent