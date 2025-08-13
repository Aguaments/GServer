#include "http_server.h"

#include "http_session.h"
namespace agent{
    namespace http{
        static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

        HttpServer::HttpServer(bool keepalive
                    , IOManager* worker
                    , IOManager* accept_worker)
            :TcpServer(worker, accept_worker)
            ,m_isKeepalive(keepalive) {
        }

        void HttpServer::handleClient(Socket::ptr client){
            HttpSession::ptr session(new HttpSession(client));
            do{
                auto req  = session -> recvRequest();
                if(!req){
<<<<<<< HEAD
                    AGENT_LOG_WARN(g_logger) << "recv http request fail, errno=" << errno 
                                             << " errstr=" << strerror(errno)
                                             << " client: " << *client;
=======
                    // AGENT_LOG_WARN(g_logger) << "recv http request fail, errno=" << errno 
                    //                          << " errstr=" << strerror(errno)
                    //                          << " client: " << *client;
>>>>>>> f0ef15c (rebuild repository after corruption)
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req -> getVersion(), req -> isClose() || !m_isKeepalive));
                rsp ->setBody("hello agent");

                // AGENT_LOG_DEBUG(g_logger) << "request: " << std::endl <<  *req;
                // AGENT_LOG_DEBUG(g_logger) << "response: " << std::endl <<  *rsp;
                AGENT_LOG_INFO(g_logger) << "[HandleClient]"
                                         << " [Request]: " << *req
                                         << " [Response] " << *rsp;
                session->sendResponse(rsp);
                
            }while(m_isKeepalive);
            session->close();
        }   
    }
}