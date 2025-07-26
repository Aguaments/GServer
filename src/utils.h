#pragma once

#include <vector>
#include <string>

#include <stdint.h>
#include <sys/types.h> 

namespace agent{
    class Utils{
    public:
        static pid_t getThreadId();   
        static int32_t getCoroutineId();

        static std::string BacktraceToString(int size, int skip = 0, const std::string& prefix = "");

    private:
        static void Backtrace(std::vector<std::string>& bt, int size, int skip = 0);
        
    };
}