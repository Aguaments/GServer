#include <sstream>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>

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
    std::cout << std::hex << (1 << (sizeof(uint32_t) * 8 - 11)) << std::endl;
    return 0;
}