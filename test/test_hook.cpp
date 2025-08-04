#include "hook.h"
#include "iomanager.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();
agent::IOManager iom(1, false, "test", true);

void test_sleep(){
    
    iom.schedule([](){
        sleep(5);
        AGENT_LOG_INFO(g_logger) << "sleep 2";
    });
    iom.schedule([](){
        sleep(6);
        AGENT_LOG_INFO(g_logger) << "sleep 3";
    });
    AGENT_LOG_INFO(g_logger) << "sleep sleep";
}

int main(){
    // agent::IOManager iom(1, false, "test", true);
    test_sleep();
    return 0;
}