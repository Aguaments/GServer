#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

int count = 0;
agent::Spinlock s_mutex;
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
        agent::Spinlock::Lock lock(s_mutex);
        std::stringstream ss;
        ss << "thread_id = <<" << agent::Utils::getThreadId() << ">>";
        AGENT_LOG_INFO(g_logger) <<  ss.str();
    }
}
void func3()
{
    for(int i = 0 ; i < 100; ++ i)
    {
        agent::Spinlock::Lock lock(s_mutex);
        std::stringstream ss;
        ss << "================ thread_id = <<" << agent::Utils::getThreadId() << ">>";
        AGENT_LOG_INFO(g_logger) <<  ss.str();
    }
}

int main()
{
    // AGENT_LOG_INFO(g_logger) << "thread test begin";
    // agent::Thread::ptr thr2(new agent::Thread(&func3, "name_" + std::to_string(2)));
    // agent::Thread::ptr thr1(new agent::Thread(&func2, "name_" + std::to_string(1)));
    
    
    // thr2 -> join();
    // thr1 -> join();

    // AGENT_LOG_INFO(g_logger) << "thread test end";

    // AGENT_LOG_INFO(g_logger) << "count=" << count; 

    YAML::Node root = YAML::LoadFile("../config/log.yml");
    agent::Config::LoadFromYaml(root);

    agent::Config::Visit([](agent::ConfigVarBase::ptr& var){
        AGENT_LOG_INFO(AGENT_LOG_ROOT()) << "name=" << var -> getName()
                    << " description=" << var -> getDescription()
                    << " typename=" << var -> getTypeName()
                    << " value=" << var -> toString();
    });
    return 0;
}