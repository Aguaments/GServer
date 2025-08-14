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

    auto sd = server -> getServletDispatch();
    sd -> addGlobServlet("/agent/*", [](agent::http::HttpRequest::ptr req, agent::http::HttpResponse::ptr rsp, agent::http::HttpSession::ptr session) -> int32_t{
        rsp -> setBody(req -> toString());
        return 0;
    });
    server -> start();
}

int main(){
    agent::IOManager iom(7);
    iom.schedule(run);

    // sleep(100000);
    return 0;
}


