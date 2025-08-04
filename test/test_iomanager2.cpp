#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "agent.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

int fd = 0;

void test_coroutine(){

    fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(23);
    inet_pton(AF_INET, "192.168.138.129", &addr.sin_addr.s_addr);
    
    if(!connect(fd, (sockaddr*)&addr, sizeof(addr))){
        AGENT_LOG_ERROR(g_logger) << "connect error";
    }else if(errno == EINPROGRESS){
        // agent::IOManager::GetThis() -> addEvent(fd, agent::IOManager::READ, [](){
        //     AGENT_LOG_INFO(g_logger) << "read Callback";
        // });

        agent::IOManager::GetThis() -> addEvent(fd, agent::IOManager::WRITE, [](){
            AGENT_LOG_INFO(g_logger) << "write Callback";
            agent::IOManager::GetThis() -> cancelEvent(fd, agent::IOManager::READ);
            close(fd);
        });
    }

}

void test(){
    AGENT_LOG_INFO(g_logger) << " test ------------";
}

void test_timer(){
    agent::IOManager iom(2, false, "test", true);
    iom.addTimer(500, [](){
        AGENT_LOG_INFO(g_logger) << "timer....";
    }, true);
}

int main()
{
    // agent::IOManager iom(2, false, "test", true);
    // iom.schedule(&test);
    // iom.schedule(&test_coroutine);
    agent::IOManager iom(2, false, "test", true);
    iom.addTimer(5000, [](){
        AGENT_LOG_INFO(g_logger) << "timer....";
    }, true);
    // iom.addTimer(5000, [](){
    //     AGENT_LOG_INFO(g_logger) << "timer2....";
    // }, true);
    iom.schedule(test);
    // sleep(10000);
    return 0;

}