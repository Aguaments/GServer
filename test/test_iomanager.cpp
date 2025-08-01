#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();
agent::IOManager::ptr iom = std::make_shared<agent::IOManager>(2, false, "test", true);

void test_coroutine(){
    for(int i = 0; i < 5; ++ i){
        AGENT_LOG_INFO(g_logger)<< "test_coroutine thread " << agent::Utils::getThreadId();
        sleep(1);
    }
    
}

int main(int argc, char** argv){
    iom -> schedule(&test_coroutine);
    iom -> schedule(&test_coroutine);
    iom -> schedule(&test_coroutine);
    //iom -> stop();
    while(true);
    return 0;
}