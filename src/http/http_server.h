#pragma once

#include <memory>

#include "iomanager.h"
#include "tcp_server.h"
#include "socket.h"


namespace agent{
    namespace http{
        class HttpServer: public TcpServer{
        public:
            using ptr = std::shared_ptr<HttpServer>;
        
            HttpServer(bool keepalive = false
                        , IOManager* worker = IOManager::GetThis()
                        , IOManager* accept_worker = IOManager::GetThis());
        protected:
            virtual void handleClient(Socket::ptr client) override;
        
        private:
            bool m_isKeepalive;
        };
    }
}