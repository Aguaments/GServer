#pragma once

#include <stdint.h>

#include <sys/types.h> 

namespace agent{
    class Utils{
    public:
        static pid_t getThreadId();   
        static int32_t getCoroutineId();
    };
}