#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

int count = 0;

void fun1()
{
    
    AGENT_LOG_INFO(g_logger) << "name: " << agent::Thread::GetName()
                             << " this.name: " << agent::Thread::GetThis() -> getName()
                             << " id:" << agent::Utils::getThreadId()
                             << " this.id: " << agent::Thread::GetThis() -> getId(); 
    for(int i = 0; i < 1000000 ; ++ i)
    {
        ++count;
    }
}

int main()
{
    AGENT_LOG_INFO(g_logger) << "thread test begin";
    std::vector<agent::Thread::ptr> thrs;
    for(int i = 0 ; i < 5; ++ i)
    {
        agent::Thread::ptr thr(new agent::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }
    
    for(int i = 0; i < 5; ++ i)
    {
        thrs[i] -> join();
    }
    AGENT_LOG_INFO(g_logger) << "thread test end";

    AGENT_LOG_INFO(g_logger) << "count=" << count; 
    return 0;
}