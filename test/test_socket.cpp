#include "socket.h"
#include "agent.h"


static agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void test_socket(){
    agent::IPAddress::ptr addr = agent::Address::LookupAnyIPAddress("127.0.0.1");
    if(addr){
        AGENT_LOG_INFO(g_logger) << "get address " << addr -> toString();
    }else{
        AGENT_LOG_ERROR(g_logger) << "get address fail.";
        return;
    }

    agent::Socket::ptr sock = agent::Socket::CreateTCP(addr);
    addr -> setPort(8000);
    AGENT_LOG_INFO(g_logger) << addr -> toString();
    if(!sock -> connect(addr)){
        AGENT_LOG_ERROR(g_logger) << "connect " << addr -> toString() << " fail";
        return ;
    }else{
        AGENT_LOG_INFO(g_logger) << "connect " << addr -> toString() << " connected";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock -> send(buff, sizeof(buff));
    if(rt <= 0){
        AGENT_LOG_INFO(g_logger) << "send fail rt = " << rt;
        return;
    }

    std::cout << "hhh" << std::endl;
    std::string buffers;
    buffers.resize(4096);
    rt = sock -> recv(&buffers[0], buffers.size());

    if(rt <= 0){
        AGENT_LOG_INFO(g_logger) << "send fail rt = " << rt;
        return ;
    }

    buffers.resize(rt);
    AGENT_LOG_INFO(g_logger) << buffers;
    
}

int main(){
    // agent::IOManager iom;
    test_socket();
    return 0;
}