#pragma once

#include <memory>
#include <set>

#include "thread.h"

namespace agent{

    class TimerManager;

    class Timer: public std::enable_shared_from_this<Timer>{
        friend class TimerManager;
    public:
        using ptr = std::shared_ptr<Timer>;
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

        uint64_t getNextTimer();

        void listExpiredCb(std::vector<std::function<void()>>& cbs);
    
    protected:
        virtual void onTimerInsertedAtFront() = 0;


    private:
        RWMutexType m_mutex;
        std::set<Timer::ptr, Timer::Comparator> m_timers; 
    };
}