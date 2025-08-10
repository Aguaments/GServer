#pragma once

#include <assert.h>
#include <string.h>
#include "log.h"
#include "utils.h"

#if defined __GNUC__ || defined __llvm__
#   define AGENT_LIKELY(x) __builtin_expect(!!(x), 1)
#   define AGENT_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define AGENT_LIKELY(x) (x)
#   define AGENT_UNLIKELY(x) (x)
#endif


#define AGENT_ASSERT(x) \
    if(AGENT_UNLIKELY(!(x))) \
    { \
        AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << agent::Utils::BacktraceToString(100, 0, "    "); \
        assert(x); \
    }

#define AGENT_ASSERT_PARA(x, w) \
    if(AGENT_UNLIKELY(!(x))) \
    { \
        AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << agent::Utils::BacktraceToString(100, 0, "    "); \
            assert(x); \
    }
