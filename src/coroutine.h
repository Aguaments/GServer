#pragma once

#include <memory>
#include <functional>

#include <ucontext.h>

#include "thread.h"
// #include "scheduler.h"

namespace agent{
    class Scheduler;
    class Coroutine: public std::enable_shared_from_this<Coroutine>
    {
        friend class Scheduler;
    public:
        using ptr = std::shared_ptr<Coroutine>;
        
        enum class State
        {
            INIT,
            HOLD,
            EXEC,
            TERM,
            READY,
            EXCEPT
        };
        
    private:
        Coroutine();
    public:
        Coroutine(std::function<void()> cb, size_t stacksize = 0, bool user_caller = true, std::string name = "");
        ~Coroutine();

        // 重置协程函数，在init中或者term状态
        void reset(std::function<void()> cb);
        // 切换到当前协程执行
        void swapIn();
        // 切换到后台执行
        void swapOut();

        void call();
        void back();

        uint64_t getId() const {return m_id;}
        const State getState() const {return m_state;}
        const std::string getName() const {return m_name;}
        // void setState(State state) {m_state = state;}

    public:
        // 协程切换到后台，设置ready状态（等待被调度器调用的协程会处于ready状态）
        static void YieldToReady();
        // 协程切换到后台，设置hold状态（不受调度器控制的协程，需要手动恢复的协程）
        static void YieldToHold();
        // 协程执行的主函数
        static void MainFunc();
        static void CallerMainFunc();

        // 设置当前协程
        static void SetThis(Coroutine* cor);
        // 返回当前协程
        static Coroutine::ptr GetThis();
        // 返回协程总数
        static uint64_t TotalCoroutine();

        static uint64_t GetCoroutineId();

        
    
    private:
        uint64_t m_id = 0; // 协程id
        uint32_t m_stacksize = 0;
        State m_state = State::INIT;

        ucontext_t m_ctx;
        void* m_stack = nullptr;

        std::function<void()> m_cb;

        std::string m_name;
    };
}