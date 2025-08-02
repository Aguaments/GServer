#include "timer.h"

#include "utils.h"

namespace agent{
    bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const{
        if(!lhs && !rhs) {
            return false;
        }
        if(!rhs){
            return true;
        }
        if(!rhs){
            return false;
        }
        if(lhs -> m_next < rhs -> m_next){
            return true;
        }
        if(rhs -> m_next < lhs -> m_next){
            return false;
        }

        return lhs.get() < rhs.get();
    }

    Timer::Timer(uint64_t ms, std::function<void()> cb, bool recuring, TimerManager* manager)
    :m_recuring(recuring)
    ,m_ms(ms)
    ,m_cb(cb)
    ,m_manager(manager){
        m_next = Utils::GetCurrentMS() + m_ms; // 具体的执行时间
    }


    TimerManager::TimerManager(){

    }
    TimerManager::~TimerManager(){

    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recuring){
        Timer::ptr timer(new Timer(ms, cb, recuring, this));
        RWMutexType::WriteLock lock(m_mutex);
        auto it = m_timers.insert(timer).first;
        bool at_front = (it == m_timers.begin());
        lock.unlock();

        if(at_front){
            onTimerInsertedAtFront();
        }
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb){
        std::shared_ptr<void> tmp = weak_cond.lock();
        if(tmp){
            cb();
        }
    }

    Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond, bool recuring){
        return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recuring);
    }

    uint64_t TimerManager::getNextTimer(){
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()){
            return ~0ull;
        }

        const Timer::ptr& next = *m_timers.begin();
        uint64_t now_ms = Utils::GetCurrentMS();
        if(now_ms >= next -> m_next){
            return 0;
        }else{
            return next -> m_next - now_ms;
        }
    }   

    void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs){
        uint64_t now_ms = Utils::GetCurrentMS();
        std::vector<Timer::ptr> expired;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if(m_timers.empty()){
                return ;
            }
        }
    }
}