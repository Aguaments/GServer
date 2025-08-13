#pragma once

#include "http.h"
#include "socket_stream.h"
#include "socket.h"

namespace agent{
    namespace http{
        class HttpSession: public SocketStream{
        public:
            using ptr = std::shared_ptr<HttpSession>;
            HttpSession(Socket::ptr sock, bool owner = true);

            HttpRequest::ptr recvRequest();
            int sendResponse(HttpResponse::ptr rsp);
        };
    }
}