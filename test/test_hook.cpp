#include "hook.h"
#include "iomanager.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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


void test_sock(){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "192.168.138.129", &addr.sin_addr.s_addr);

    int rt = connect(sock, (sockaddr*)&addr, sizeof(addr));
    AGENT_LOG_DEBUG(g_logger) << "connect rt=" << rt << " errno=" << errno;
    if(rt){
        return;
    }

    const char data[] =  "GET /Makefile HTTP/1.0\r\n\r\n";
    rt =send(sock, data, sizeof(data),0);
    AGENT_LOG_DEBUG(g_logger) << "connect rt=" << rt << " errno=" << errno;
    if(rt <= 0){
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    if(rt <= 0){
        return;
    }
    buff.resize(rt);
    AGENT_LOG_DEBUG(g_logger) << buff;
}

int main(){
    // agent::IOManager iom(1, false, "test", true);
    test_sock();
    return 0;
}