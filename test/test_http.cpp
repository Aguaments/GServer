#include "http/http.h"
#include "log.h"

void test_request(){
    agent::http::HttpRequest::ptr req(new agent::http::HttpRequest);
    req -> setHeader("host", "www.baidu.com");
    req -> setBody("hello agent");

    req -> dump(std::cout) << std::endl;
}

void test_response(){
    agent::http::HttpResponse::ptr req(new agent::http::HttpResponse);
    req -> setHeader("X-X", "agent");
    req -> setBody("hello agent");

    req -> dump(std::cout) << std::endl;
}

int main(){
    test_request();
    test_response();
    return 0;
}