#pragma once

#include <memory>

#include "iomanager.h"
#include "tcp_server.h"
#include "socket.h"
#include "servlet.h"


namespace agent{
    namespace http{
        class HttpServer: public TcpServer{
        public:
            using ptr = std::shared_ptr<HttpServer>;
        
            HttpServer(bool keepalive = false
                        , IOManager* worker = IOManager::GetThis()
                        , IOManager* accept_worker = IOManager::GetThis());

            ServletDispatch::ptr getServletDispatch() const {return m_dispatch;}
            void setServletDispatch(ServletDispatch::ptr v) {m_dispatch = v;}
            
        protected:
            virtual void handleClient(Socket::ptr client) override;
        
        private:
            bool m_isKeepalive;
            ServletDispatch::ptr m_dispatch;
        };
    }
}