#include "address.h"
#include <sstream>

#include <arpa/inet.h>
#include <netdb.h>

#include "endian.hpp"
#include "log.h"

namespace agent {

    static Logger::ptr g_logger = AGENT_LOG_ROOT();

    template<class T>
    static T CreateMask(uint32_t bits){
        return (1 << (sizeof(T) * 8 - bits)) - 1;
    }

    Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen){
        if(addr == nullptr){
            return nullptr;
        }

        Address::ptr result;
        // switch(m_addr)
        switch(addr->sa_family) {
            case AF_INET:
                result.reset(new IPv4Address(*(const sockaddr_in*)addr));
                break;
            case AF_INET6:
                result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
                break;
            default:
                result.reset(new UnknowAddress(*addr));
                break;
        }
        return result;
    }

    bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host, int family, int type, int protocol) {
        addrinfo hints, *results, *next;
        hints.ai_flags = 0;
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;
        hints.ai_addrlen = 0;
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        std::string node;
        const char* service = NULL;

        //检查 ipv6address serivce
        if(!host.empty() && host[0] == '[') {
            const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
            if(endipv6) {
                //TODO check out of range
                if(*(endipv6 + 1) == ':') {
                    service = endipv6 + 2;
                }
                node = host.substr(1, endipv6 - host.c_str() - 1);
            }
        }

        //检查 node serivce
        if(node.empty()) {
            service = (const char*)memchr(host.c_str(), ':', host.size());
            if(service) {
                if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                    node = host.substr(0, service - host.c_str());
                    ++service;
                }
            }
        }

        if(node.empty()) {
            node = host;
        }
        int error = getaddrinfo(node.c_str(), service, &hints, &results);
        if(error) {
            AGENT_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ", "
                << family << ", " << type << ") err=" << error << " errstr="
                << gai_strerror(error);
            return false;
        }

        next = results;
        while(next) {
            result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
            //SYLAR_LOG_INFO(g_logger) << ((sockaddr_in*)next->ai_addr)->sin_addr.s_addr;
            next = next->ai_next;
        }

        freeaddrinfo(results);
        return !result.empty();
    }

    int Address::getFamily() const {
        return getAddr() -> sa_family;
    }

    std::string Address::toString(){
        std::stringstream ss;
        insert(ss);
        return ss.str();
    }



    bool Address::operator<(const Address& rhs) const{
        socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
        int result = memcmp(getAddr(), rhs.getAddr(), minlen);
        if(result < 0){
            return true;
        }else if(result > 0){
            return false;
        }else if(getAddrLen() < rhs.getAddrLen()){
            return true;
        }
        return false;
    }

    bool Address::operator!=(const Address& rhs) const{
        return !(*this == rhs);    
    }

    IPAddress::ptr IPAddress::Create(const char* address, uint32_t port){
        addrinfo hints, *results;
        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_flags = AI_NUMERICHOST;
        hints.ai_family = AF_UNSPEC;

        int error = getaddrinfo(address, NULL, &hints, &results);
        if(error){
            AGENT_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                      << port << ") rt = " << results << " errno = " << errno
                                      << " errstr = " << strerror(errno);
            return nullptr;
        }

        try{
            IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(
                Address::Create(results -> ai_addr, (socklen_t)results -> ai_addrlen)
            );
            if(result){
                result -> setPort(port);
            }

            freeaddrinfo(results);
            return result;
        }catch(...){
            freeaddrinfo(results);
            return nullptr;
        }
    }

    IPv4Address::IPv4Address(){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin_family = AF_INET;
    }

    IPv4Address::IPv4Address(const sockaddr_in& address){
        m_addr = address;
    }

    IPv4Address::IPv4Address(uint32_t address, uint32_t port){
        memset(&address, 0, sizeof(address));
        m_addr.sin_family = address;
        m_addr.sin_port = byteswapOnLittleEndian(port);
        m_addr.sin_addr.s_addr = byteswapOnLittleEndian(address);
    }

    const sockaddr* IPv4Address::getAddr() const{
        return (sockaddr*)&m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const {
        return sizeof(m_addr);
    }

    std::ostream& IPv4Address::insert(std::ostream& os) const {
        uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
        os << ((addr >> 24) & 0xff) << "."
           << ((addr >> 16) & 0xff) << "."
           << ((addr >> 8) & 0xff) << "."
           << (addr & 0xff);
        os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
        return os;
    }

    IPv4Address::ptr IPv4Address::Create(const char* address, uint32_t port){
        IPv4Address::ptr rt(new IPv4Address);
        rt -> m_addr.sin_port = byteswapOnLittleEndian(port);
        int result = inet_pton(AF_INET, address, &rt -> m_addr.sin_addr.s_addr);
        if(result < 0){
            AGENT_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                      << port << ") rt = " << result << " errno = " << errno
                                      << " errstr = " << strerror(errno);
            return nullptr;
        }
        return rt;
    }

    // 广播地址为网络号不变 主机号全1
    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len){
        if(prefix_len > 32){
            return nullptr;
        }

        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        
        return IPv4Address::ptr(new IPv4Address(baddr));
    }

    IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len){
        if(prefix_len > 32){
            return nullptr;
        }

        sockaddr_in baddr(m_addr);
        baddr.sin_addr.s_addr &= byteswapOnLittleEndian(~CreateMask<uint32_t>(prefix_len));
        
        return IPv4Address::ptr(new IPv4Address(baddr));
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len){
        sockaddr_in subnet;
        memset(&subnet, 0, sizeof(subnet));
        subnet.sin_family = AF_INET;
        subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
        return IPv4Address::ptr(new IPv4Address(subnet));
    }

    void IPv4Address::setPort(uint32_t port){
        m_addr.sin_port = byteswapOnLittleEndian(port);
    }

    IPv6Address::IPv6Address(){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
    }

    IPv6Address::IPv6Address(const sockaddr_in6& address){
        m_addr = address;
    }

    IPv6Address::IPv6Address(const uint8_t address[16], uint32_t port){
        memset(&address, 0, 16);
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = byteswapOnLittleEndian(port);
        memcpy(&m_addr.sin6_addr.__in6_u, address, 16);
    }

    std::ostream& IPv6Address::insert(std::ostream& os) const{
        os << "[";
        uint16_t* addr = const_cast<uint16_t*>(m_addr.sin6_addr.__in6_u.__u6_addr16);
    
        bool use_zero = false;
        for(size_t i = 0; i < 8; ++ i){
            if(addr[i] == 0 && !use_zero){
                continue;
            }
            if(1 && addr[i] == 0 && !use_zero){
                os << ":";
                use_zero = true;
            }
            if(i){
                os << ":";
            }
            os << std::hex << (int)byteswapOnLittleEndian(addr[i]);
        }
        if(!use_zero && addr[7] == 0){
            os << "::";
        }

        os << "]" << byteswapOnLittleEndian(m_addr.sin6_port);
        return os;
    }

    IPv6Address::ptr IPv6Address::Create(const char* address, uint32_t port){
        IPv6Address::ptr rt(new IPv6Address);
        rt -> m_addr.sin6_port = byteswapOnLittleEndian(port);
        int result = inet_pton(AF_INET6, address, &rt -> m_addr.sin6_addr.__in6_u);
        if(result < 0){
            AGENT_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                      << port << ") rt = " << result << " errno = " << errno
                                      << " errstr = " << strerror(errno);
            return nullptr;
        }
        return rt;
    }


    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len){
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
        for(int i = (int)prefix_len / 8 + 1; i < 16; ++i){
            baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }

    IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len){
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] &= ~CreateMask<uint8_t>(prefix_len % 8);
        for(int i = (int)prefix_len / 8 + 1; i < 16; ++i){
            baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0x00;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }
    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len){
        sockaddr_in6 baddr(m_addr);
        baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len/ 8] = ~CreateMask<uint8_t>(prefix_len % 8);
        for(int i = 15; i >(int)prefix_len / 8; --i){
            baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0x00;
        }
        for(int i = 0; i < (int)prefix_len/ 8; ++ i){
            baddr.sin6_addr.__in6_u.__u6_addr8[i] = 0xff;
        }
        return IPv6Address::ptr(new IPv6Address(baddr));
    }

    uint32_t IPv6Address::getPort() const{
        return byteswapOnLittleEndian(m_addr.sin6_port);
    }

    void IPv6Address::setPort(uint32_t port) {
        m_addr.sin6_port = byteswapOnLittleEndian(port);
    }

    static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0) -> sun_path) - 1;

    UnixAddress::UnixAddress(){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
    }

    UnixAddress::UnixAddress(const std::string& path){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sun_family = AF_UNIX;
        m_length = path.size() + 1;

        if(!path.empty() && path[0] == '\0'){
            --m_length;
        }

        if(m_length <= sizeof(m_addr.sun_path)){
            throw std::logic_error("path too log");
        }
        memcpy(m_addr.sun_path, path.c_str(), m_length);
        m_length += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr* UnixAddress::getAddr() const{
        return (sockaddr*)&m_addr;
    }

    socklen_t UnixAddress::getAddrLen() const{
        return m_length;
    }

    std::ostream& UnixAddress::insert(std::ostream& os) const{
        if(m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0'){
            return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);
        }
        return os << m_addr.sun_path;
    }

    UnknowAddress::UnknowAddress(int family){
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sa_family = family;
    }

    UnknowAddress::UnknowAddress(const sockaddr& addr){
        m_addr = addr;
    }

    const sockaddr* UnknowAddress::getAddr() const{
        return &m_addr;
    }
    socklen_t UnknowAddress::getAddrLen() const{
        return sizeof(m_addr);
    }
    std::ostream& UnknowAddress::insert(std::ostream& os) const{
        os << "[UnknowAddress family=" << m_addr.sa_family << "]";
        return os;
    }
}