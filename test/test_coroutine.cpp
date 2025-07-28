#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();


void run_in_coroutine()
{


    AGENT_LOG_INFO(g_logger) << "run in coroutine begin";
    agent::Coroutine::GetThis() -> YieldToHold();
    AGENT_LOG_INFO(g_logger) << "run in coroutine end";
}

void test_coroutine()
{
    agent::Coroutine::GetThis();
    AGENT_LOG_INFO(g_logger) << "main begin"; 
    agent::Coroutine::ptr cor(new agent::Coroutine(run_in_coroutine));
    cor -> swapIn();
    AGENT_LOG_INFO(g_logger)<< "main after swapIn";
    cor -> swapIn();
    AGENT_LOG_INFO(g_logger) << "main after end";
}

int main(int argc, char** argv)
{
    agent::Thread::SetName("MainThread");
    std::vector<agent::Thread::ptr> ths;
    for(int i = 0; i < 3; ++ i)
    {
        ths.push_back(agent::Thread::ptr(new agent::Thread(test_coroutine, "name_" + std::to_string(i))));
    }
    for(auto& th: ths)
    {
        th -> join();
    }
    AGENT_LOG_INFO(g_logger) << "main after end";
    return 0;
}