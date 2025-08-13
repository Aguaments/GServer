#include <unistd.h>

#include "tcp_server.h"
#include "iomanager.h"
#include "log.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void run(){
    auto addr = agent::Address::LookupAny("127.0.0.1:10009");
    AGENT_LOG_WARN(g_logger) << addr -> toString();
    // auto addr2 = agent::UnixAddress::ptr(new agent::UnixAddress("/tmp/unix_addr"));
    std::vector<agent::Address::ptr> addrs;
    addrs.push_back(addr);
    // addrs.push_back(addr2);

    agent::TcpServer::ptr tcp_server(new agent::TcpServer);
    std::vector<agent::Address::ptr> fails;
    while(!tcp_server -> bind(addrs, fails)){
        sleep(2);
    }
    tcp_server -> start();
}

int main(){
    agent::IOManager iom(2);
    iom.schedule(run);
    return 0;
}