#include "http_server.h"

#include "http_session.h"
namespace agent{
    namespace http{
        static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

        HttpServer::HttpServer(bool keepalive, IOManager* worker, IOManager* accept_worker)
        :TcpServer(worker, accept_worker)
        ,m_isKeepalive(keepalive) {
            m_dispatch.reset(new ServletDispatch);
        }

        void HttpServer::handleClient(Socket::ptr client){
            HttpSession::ptr session(new HttpSession(client));
            do{
                auto req  = session -> recvRequest();
                if(!req){

                    break;
                }
                // AGENT_LOG_INFO(g_logger) << "[Request] "
                // << HttpMethodToString(req -> getMethod()) << " " 
                // << req -> getPath() << " "
                // << "HTTP/" 
                // << ((uint32_t)(req -> getVersion() >> 4))
                // << "." << ((uint32_t)(req -> getVersion() & 0x0F)) << " "
                // << HttpStatusToString(req -> getStatus());
                

                HttpResponse::ptr rsp(new HttpResponse(req -> getVersion(), req -> isClose() || !m_isKeepalive));
                m_dispatch -> handle(req, rsp, session);
                session->sendResponse(rsp);
                
            }while(m_isKeepalive);
            session->close();
        }   
    }
}