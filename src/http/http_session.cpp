#include "http_session.h"

#include <memory>

#include "http_parser.h"
#include "log.h"

namespace agent{
    namespace http{
        static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");
        HttpSession::HttpSession(Socket::ptr sock, bool owner)
        :SocketStream(sock, owner){}

        HttpRequest::ptr HttpSession::recvRequest(){
            HttpRequestParser::ptr parser(new HttpRequestParser);
            uint64_t req_buff_size = HttpRequestParser::GetHttpRequestBufferSize();

            //AGENT_LOG_ERROR(g_logger) << "buff size " << req_buff_size;

            std::shared_ptr<char> buff(new char[req_buff_size], [](char* ptr){delete[] ptr;});

            char*  buff_raw_ptr = buff.get();

            int unresolve_bytes = 0;

            do{
                // [----unresolve space----|----------free space----------]
                //                         ^
                //                         |
                //                      unresolve_bytes 
                int len = read(buff_raw_ptr + unresolve_bytes, req_buff_size - unresolve_bytes);
                if(len <= 0){
                    close();
                    return nullptr;
                }

                // [----unresolve space----|----len----|----free space----]
                //                         ^
                //                         |
                //                      unresolve_bytes 

                len += unresolve_bytes; // 待解析的实际长度
                // 实际解析的长度
                // execute会解析到长度最大的符合解析要求的位置
                // 如果完整的读取了请求体并且没有错误，那么解析器会在read执行的当前循环中解析完毕，不会在进入下次循环
                size_t nparse = parser -> execute(buff_raw_ptr, len); 

                // [----unresolve space----|----len----|----free space----]
                //       
                //  解析的长度会被后续的数据覆盖，转变为如下的(解析了nparse)
                //
                // [--unresolve--|--------------free space----------------]
                if(parser -> hasError()){
                    close();
                    return nullptr;
                }
                unresolve_bytes = len - nparse;
                if(unresolve_bytes == (int)req_buff_size){
                    close();
                    return nullptr;
                }
                if(parser -> isFinished()){
                    break;
                }

            }while(true);

            int64_t length = parser -> getContentLength(); // 获取请求体的长度
            if(length > 0){
                std::string body;
                body.resize(length);

                int len = 0;
                if(length >= unresolve_bytes){
                    // 如果长度大于等用户未解析的字节数，就直接把数据写入body
                    memcpy(&body[0], buff_raw_ptr, unresolve_bytes);
                    len = unresolve_bytes;
                }else{
                    memcpy(&body[0], buff_raw_ptr, length);
                    len = length;
                }
                length -= unresolve_bytes;
                if(length > 0){
                    if(readFixSize(&body[len], length) <= 0){
                        close();
                        return nullptr;
                    }
                }
                parser -> getData() -> setBody(body);
            }
            return parser -> getData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp){
            std::stringstream ss;
            ss << *rsp;
            std::string data = ss.str();
            return writeFixSize(data.c_str(), data.size());
        }
    }
}
