#include "http/http_server.h"

#include <unistd.h>

#include "log.h"

static agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void run(){
    agent::http::HttpServer::ptr server(new agent::http::HttpServer);
    agent::Address::ptr addr = agent::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server -> bind(addr)){
        sleep(2);
    }

    server -> start();
}

int main(){
    agent::IOManager iom(8);
    iom.schedule(run);
    sleep(10000);
    return 0;
}


