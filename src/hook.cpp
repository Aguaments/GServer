#include "hook.h"

#include <functional>

#include <dlfcn.h>

#include "coroutine.h"
#include "iomanager.h"
#include "scheduler.h"
#include "log.h"
#include "fd_manager.h"

namespace agent{
    
    static thread_local bool t_hook_enable = false;
    agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    #define HOOK_FUNC(XX) \
        XX(sleep) \
        XX(usleep) \
        XX(nanosleep)\
        XX(socket) \
        XX(accept) \
        XX(bind) \
        XX(listen) \
        XX(connect) \
        XX(read) \
        XX(readv) \
        XX(recv) \
        XX(recvfrom) \
        XX(recvmsg) \
        XX(write) \
        XX(send) \
        XX(sendto) \
        XX(sendmsg) \
        XX(close) \
        XX(fcntl) \
        XX(ioctl) \
        XX(getsockopt) \
        XX(setsockopt) 


    void hook_init(){
        static bool is_inited = false;
        if(is_inited){
            return ;
        }
        #define XX(name) name ## _f = (name ## _func)dlsym(RTLD_NEXT, #name);
            HOOK_FUNC(XX);
        #undef XX
    }

    struct HookIniter{
        HookIniter(){
            hook_init();
        }
    };

    bool is_hook_enable(bool flag){
        return t_hook_enable;
    }

    void set_hook_enable(bool flag){
        t_hook_enable = flag;
    }
}

#ifdef __cplusplus
extern "C"{
#endif

    #define XX(name) name ## _func name ## _f = nullptr;
        HOOK_FUNC(XX);
    #undef XX

    struct timer_info{
        int cancelled = 0;
    };

    template<typename OriginFun, typename... Args>
    static ssize_t do_io(int fd, OriginFun fun, const char* hook_fun_name, uint32_t event, int timeout_so, Args&&... args){
        if(!agent::t_hook_enable){
            return fun(fd,std::forward<Args>(args)...);
        }

        agent::FdCtx::ptr ctx = agent::FdMgr::getInstance() -> get(fd);
        if(!ctx){
            return fun(fd, std::forward<Args>(args)...);
        }

        if(ctx -> isClose()){
            errno = EBADF;
            return -1;
        }

        if(!ctx -> isSocket() || ctx -> getUserNonblock()){
            return fun(fd, std::forward<Args>(args)...);
        }

        uint64_t to = ctx -> getTimeout(timeout_so);
        std::shared_ptr<timer_info> tinfo(new timer_info);

    retry:
        ssize_t n = fun(fd, std::forward<Args>(args)...);
        while(n == -1 && errno == EINTR){
            n = fun(fd, std::forward<Args>(args)...);
        }
        if(n == -1 && errno == EAGAIN){
            agent::IOManager* iom = agent::IOManager::GetThis();
            agent::Timer::ptr timer;
            std::weak_ptr<timer_info> winfo(tinfo);

            if(to != (uint64_t)-1){
                timer = iom -> addConditionTimer(to, [winfo, fd, iom, event](){
                    auto t = winfo.lock();
                    if(!t || t -> cancelled){
                        return ;
                    }
                    t -> cancelled = ETIMEDOUT;
                    iom -> cancelEvent(fd, (agent::IOManager::EventType)(event));
                }, winfo);
            }

            int rt = iom -> addTimer(fd, (agent::IOManager::EventType)(event));
            if(rt){
                AGENT_LOG_ERROR(g_logger) <<  hook_fun_name << " addEvent(" << fd << ", " << event << ")";
                if(timer){
                    timer -> cancel();
                }
                return -1;
            }else{
                agent::Coroutine::YieldToHold();
                if(timer){
                    timer -> cancel();
                }
                if(tinfo -> cancelled){
                    errno = tinfo -> cancelled;
                    return -1;
                }

                goto retry;
            }
        }

        return n;
    }

    unsigned int sleep(unsigned int seconds){
        if(!agent::t_hook_enable){
            return sleep_f(seconds);
        }
        agent::Coroutine::ptr coroutine = agent::Coroutine::GetThis();
        agent::IOManager* iom = agent::IOManager::GetThis();
        iom -> addTimer(seconds * 1000, [&iom, &coroutine](){
            iom -> schedule(coroutine);
        });
        agent::Coroutine::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec){
        if(!agent::t_hook_enable){
            return usleep_f(usec);
        }
        agent::Coroutine::ptr coroutine = agent::Coroutine::GetThis();
        agent::IOManager* iom = agent::IOManager::GetThis();
        iom -> addTimer(usec / 1000, [&iom, &coroutine](){
            iom -> schedule(coroutine);
        });
        agent::Coroutine::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec * req, struct timespec* rem){
        if(!agent::t_hook_enable){
            return nanosleep_f(req, rem);
        }
        int timeout_ms = req -> tv_sec + req -> tv_nsec / 1000 / 1000;
        agent::Coroutine::ptr coroutine = agent::Coroutine::GetThis();
        agent::IOManager* iom = agent::IOManager::GetThis();

        iom -> addTimer(timeout_ms, [iom, coroutine](){
            iom -> schedule(coroutine);
        });
        agent::Coroutine::YieldToHold();
        return 0;
    }

#ifdef __cplusplus
}
#endif

using sleep_func = unsigned int (*)(unsigned int secondes);
extern sleep_func sleep_f;

using usleep_func = int (*)(useconds_t usec);
extern usleep_func usleep_f;