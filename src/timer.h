#pragma once

#include <memory>
#include <vector>
#include <set>

#include "thread.h"

namespace agent{

    class TimerManager;

    class Timer: public std::enable_shared_from_this<Timer>{
        friend class TimerManager;
    public:
        using ptr = std::shared_ptr<Timer>;

        bool cancel();
        bool refresh();
        bool reset(uint64_t ms, bool from_now);
        
    private:
        Timer(uint64_t ms, std::function<void()> cb, bool recuring, TimerManager* manager);
        Timer(uint64_t next);

    private:
        bool m_recuring = false;    // 是否循环定时器
        uint64_t m_ms = 0;          // 执行周期
        uint64_t m_next = 0;        // 精确的执行时间
        std::function<void()> m_cb;
        TimerManager* m_manager = nullptr;

    private:
        struct Comparator{
            bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
        };

    };

    class TimerManager{
    friend class Timer;
    public:
        using RWMutexType = RWMutex;

        TimerManager();
        virtual ~TimerManager();

        Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recuring = false);

        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recuring = false);

        uint64_t getNextTimer(); // 返回剩余时间

        void listExpiredCb(std::vector<std::function<void()>>& cbs); // 返回失效的定时器列表cbs

        bool hasTimer(); // 判断定时器列表中是否存在定时器
    
    protected:
        virtual void onTimerInsertedAtFront() = 0;
        void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);
    
    private:
        bool detectClockRoller(uint64_t now_ms); // 调整时间

    private:
        RWMutexType m_mutex;
        std::set<Timer::ptr, Timer::Comparator> m_timers; 
        bool m_tickled = false;
        uint64_t m_previousTime; // TimerManager创建的时间
    };
}