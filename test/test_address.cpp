#include <sstream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>

#include "log.h"
#include "address.h"

agent::Logger::ptr g_logger = AGENT_LOG_ROOT();

void test(){
    std::vector<agent::Address::ptr> addrs;

    bool v = agent::Address::Lookup(addrs, "www.baidu.com:http");
    if(!v){
        AGENT_LOG_ERROR(g_logger) << "lookup fail.";
        return ;
    }

    for(size_t i = 0; i < addrs.size(); ++ i){
        AGENT_LOG_INFO(g_logger) << i << " - " << addrs[i] -> toString();
    }
}

void test_iface(){
    std::multimap<std::string, std::pair<agent::Address::ptr, uint32_t>> results;
    bool v = agent::Address::GetInterfaceAddresses(results);
    if(!v){
        return;
    }

    for(auto& i : results){
        AGENT_LOG_INFO(g_logger) << i.first << " - " << i.second.first -> toString() << " - " << i.second.second;
    }
}

void test_ipv4(){
    auto addr = agent::IPAddress::Create("www.baidu.com");
    if(addr){
        AGENT_LOG_INFO(g_logger) << addr -> toString();
    }
}

void to_String(){
    std::stringstream os;

    struct sockaddr_in6 a;

    const char * s = "2001:0db8:0000:0000:0000:ff00:0042:8329";
    memcpy(a.sin6_addr.__in6_u.__u6_addr16, s, 16);
    auto addr = a.sin6_addr.__in6_u.__u6_addr16;

    bool use_zero = false;
    int count = 0;
    for(int i = 0; i < 8; ++ i){
        if(addr[i] == 0 && !use_zero){
            count ++;
            continue;
        }
        else{
            if(count == 1 && !use_zero){
                os << addr[i - 1];
            }
            if(count >= 2 && !use_zero){
                os << "::";
                use_zero = true;
                continue;
            }
            if(i){
                os << ":";
            }
            os << std::hex << addr[i];
        }
    }
    std::cout << os.str() << std::endl;;
}

int main(){
    test_iface();
    test_ipv4();
    return 0;
}