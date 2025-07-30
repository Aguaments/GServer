#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void test_shechduler(){
    for(int i = 0; i < 100; ++ i)
    {
        AGENT_LOG_DEBUG(g_logger) << "test scheduler";
        sleep(1);
    }
    
}

int main(int argc, char** argv)
{
    agent::Scheduler sc(3, true, "test");
    sc.schedule(test_shechduler);
    sc.schedule(test_shechduler);
    sc.schedule(test_shechduler);
    sc.start();
    // AGENT_LOG_INFO(g_logger) << "test start";
    // AGENT_LOG_INFO(g_logger) << "test end";
    sc.stop();
    return 0;
}