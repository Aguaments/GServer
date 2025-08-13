#include <unistd.h>
#include <execinfo.h>
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include "utils.h"
#include "log.h"
#include "coroutine.h"


namespace agent{

    static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    pid_t Utils::getThreadId()
    {
        static thread_local pid_t tid = ::syscall(SYS_gettid);
        return tid;
    }
    int32_t Utils::getCoroutineId()
    {
        return Coroutine::GetCoroutineId();
    }

    uint64_t Utils::GetCurrentMS(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul  + tv.tv_usec / 1000;
    }
    
    uint64_t Utils::GetCurrentUS(){
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000ul * 1000ul  + tv.tv_usec;
    }

    void Utils::Backtrace(std::vector<std::string>& bt, int size, int skip)
    {
        void** array = (void**)malloc(sizeof(void*) * static_cast<size_t>(size));
        size_t s = ::backtrace(array, size);

        char** strings = backtrace_symbols(array, s);
        if(strings == NULL)
        {
            AGENT_LOG_ERROR(g_logger) << "backtrace_symbols error";
            return;
        }

        for(size_t i = skip; i < s; ++ i)
        {
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);
    }

    std::string Utils::BacktraceToString(int size, int skip, const std::string& prefix)
    {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);

        std::stringstream ss;

        for(size_t i = skip; i < bt.size(); ++ i)
        {
            ss << prefix << bt[i] << std::endl;
        }

        return ss.str();
    }
    std::string Utils::print_epoll_events(uint32_t events) {
        
        std::stringstream ss;

        if (events & EPOLLIN)          ss << "EPOLLIN ";
        if (events & EPOLLOUT)         ss << "EPOLLOUT ";
        if (events & EPOLLPRI)         ss << "EPOLLPRI ";
        if (events & EPOLLRDHUP)       ss << "EPOLLRDHUP ";
        if (events & EPOLLERR)         ss << "EPOLLERR ";
        if (events & EPOLLHUP)         ss << "EPOLLHUP ";
        if (events & EPOLLET)          ss << "EPOLLET ";
        if (events & EPOLLONESHOT)     ss << "EPOLLONESHOT ";
        if (events & EPOLLWAKEUP)      ss << "EPOLLWAKEUP ";
        if (events & EPOLLEXCLUSIVE)   ss << "EPOLLEXCLUSIVE";

        return ss.str();
    }

}