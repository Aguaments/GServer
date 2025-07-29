#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

int main(int argc, char** argv)
{
    agent::Scheduler sc;
    sc.start();
    sc.stop();
    return 0;
}