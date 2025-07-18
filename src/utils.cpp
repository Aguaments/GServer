#include "utils.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace agent{
    int32_t Utils::getThreadId()
    {
        static thread_local int32_t tid = static_cast<int32_t>(::syscall(SYS_gettid));
        return tid;
    }
    int32_t Utils::getCoroutineId()
    {
        return 0;
    }
}