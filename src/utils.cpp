#include "utils.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace agent{
    pid_t Utils::getThreadId()
    {
        static thread_local pid_t tid = ::syscall(SYS_gettid);
        return tid;
    }
    int32_t Utils::getCoroutineId()
    {
        return 0;
    }
}