#include "agent.h"
#include <assert.h>

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void test_assert()
{
    AGENT_LOG_INFO(g_logger) << agent::Utils::BacktraceToString(10);
    AGENT_ASSERT_PARA(0 == 1, "asbsd xx");
}


int main()
{
    test_assert();
    return 0;
}