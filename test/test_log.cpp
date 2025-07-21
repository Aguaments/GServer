// #include <sstream>
// #include <string>
// #include <vector>
// #include <tuple>
// #include <iostream>
// %m -- content
// %n -- clrf
// %p -- log level
// %r -- duration (millisecond) from start
// %% -- percent
// %t -- current thread name
// %d -- current time
// %f -- file name
// %l -- line number
// %c -- class name
// %M -- method name
#include "log.h"
#include "utils.h"
#include <iostream>
#include <thread>



int main(){
    using namespace agent;
    Logger::ptr logger(new Logger());
    // logger -> setLevel(agent::LogLevel::INFO);
    LogAppender::ptr sla(new SoutLogAppender);
    logger -> addAppender(sla);
    LogAppender::ptr fla(new FileLogAppender("../log/log.txt"));
    logger -> addAppender(fla);
    for(int i = 0; i < 5; ++ i){
        AGENT_LOG_ERROR(logger) << "hello world";
    }

    AGENT_LOG_FMT_DEBUG(logger, "test fmt : %s", "TEST");

    auto lg = agent::LoggerMgr::getInstance()->getLogger("xx");
    AGENT_LOG_FMT_DEBUG(lg, "{%s.}", "hello  world 002");
    
    return 0;
}