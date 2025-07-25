#include "agent.h"
#include <thread>

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

int count = 0;
agent::NullMutex s_mutex;
// agent::Mutex s_mutex;

// void fun1()
// {
    
//     AGENT_LOG_INFO(g_logger) << "name: " << agent::Thread::GetName()
//                              << " this.name: " << agent::Thread::GetThis() -> getName()
//                              << " id:" << agent::Utils::getThreadId()
//                              << " this.id: " << agent::Thread::GetThis() -> getId(); 
//     for(int i = 0; i < 100000 ; ++ i)
//     {
//         agent::Mutex::Lock lock(s_mutex);
//         ++count;
//     }
// }

void func2()
{
    for(int i = 0 ; i < 100; ++ i)
    {
        agent::NullMutex::Lock lock(s_mutex);
        std::stringstream ss;
        ss << "thread_id = <<" << agent::Utils::getThreadId() << ">>";
        AGENT_LOG_INFO(g_logger) <<  ss.str();
    }
}
void func3()
{
    for(int i = 0 ; i < 100; ++ i)
    {
        agent::NullMutex::Lock lock(s_mutex);
        std::stringstream ss;
        ss << "================ thread_id = <<" << agent::Utils::getThreadId() << ">>";
        AGENT_LOG_INFO(g_logger) <<  ss.str();
    }
}

int main()
{
    AGENT_LOG_INFO(g_logger) << "thread test begin";
    agent::Thread::ptr thr2(new agent::Thread(&func3, "name_" + std::to_string(2)));
    agent::Thread::ptr thr1(new agent::Thread(&func2, "name_" + std::to_string(1)));
    
    
    thr2 -> join();
    thr1 -> join();

    // std::thread th(func2);
    // std::thread th2(func3);

    // th.join();
    // th2.join();

    AGENT_LOG_INFO(g_logger) << "thread test end";

    AGENT_LOG_INFO(g_logger) << "count=" << count; 
    return 0;
}