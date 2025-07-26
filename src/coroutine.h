#pragma once

#include <memory>
#include <functional>

#include <ucontext.h>

#include "thread.h"

namespace agent{
    class Coroutine: public std::enable_shared_from_this<Coroutine>
    {
    public:
        using ptr = std::shared_ptr<Coroutine>;
        
        enum class State
        {
            INIT,
            HOLD,
            EXEC,
            TERM,
            READY
        };
        
    private:
        Coroutine();


    public:
        Coroutine(std::function<void()> cb, size_t stacksize = 0);
        ~Coroutine();

        // 重置协程函数，在init中或者term状态
        void reset(std::function<void()> cb);
        // 切换到当前协程执行
        void swapIn();
        // 切换到后台执行
        void swapOut();

    public:
        // 设置当前协程
        static void SetThis(Coroutine* cor);
        // 返回当前协程
        static Coroutine::ptr GetThis();
        // 协程切换到后台，设置ready状态
        static void YieldToReady();
        // 协程切换到后台，设置hold状态
        static void YieldToHold();

        static uint64_t TotalCoroutine();

        static void MainFunc();
    
    private:
        uint64_t m_id = 0; // 协程id
        uint32_t m_stacksize = 0;
        State m_state = State::INIT;

        ucontext_t m_ctx;

        void* m_stack = nullptr;

        std::function<void()> m_cb;
    };
}