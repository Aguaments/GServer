#pragma once

#include <assert.h>
#include <string.h>
#include "log.h"
#include "utils.h"

#define AGENT_ASSERT(x) \
    if(!(x)) \
    { \
        AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ASSERTION: " #x \
            << "\nbacktrace:\n" \
            << agent::Utils::BacktraceToString(100, 0, "    "); \
        assert(x); \
    }

#define AGENT_ASSERT_PARA(x, w) \
    if(!(x)) \
    { \
        AGENT_LOG_ERROR(AGENT_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w \
            << "\nbacktrace:\n" \
            << agent::Utils::BacktraceToString(100, 0, "    "); \
            assert(x); \
    }
