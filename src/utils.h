#pragma once

#include <stdint.h>

namespace agent{
    class Utils{
    public:
        static int32_t getThreadId();   
        static int32_t getCoroutineId();
    };
}