#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void test_scheduler(){
    for(int i = 0; i < 10; ++ i)
    {
        AGENT_LOG_DEBUG(g_logger) << "test scheduler";
        sleep(1);
    }
}

int main(int argc, char** argv)
{
    agent::Scheduler::ptr sc(new agent::Scheduler(3, false, "test"));
    sc -> start();
    sc -> schedule(test_scheduler);
    sc -> schedule(test_scheduler);
    // AGENT_LOG_INFO(g_logger) << "test start";
    // AGENT_LOG_INFO(g_logger) << "test end";
    // sc.stop();

    sleep(1000);
    sc -> schedule(test_scheduler);
    return 0;
}