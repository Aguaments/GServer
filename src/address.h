#pragma once

#include <memory>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>

namespace agent{
    class Address{
    public:
        using ptr = std::shared_ptr<Address>;
        virtual ~Address(){}

        int getFamily() const;

        static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);
        static bool Lookup(std::vector<Address::ptr>& result, const std::string& host, int family = AF_INET, int type = 0, int protocol = 0);

        virtual const sockaddr* getAddr() const = 0;
        virtual socklen_t getAddrLen() const = 0;

        virtual std::ostream& insert(std::ostream& os) const = 0;
        std::string toString();

        bool operator<(const Address& rhs) const;
        bool operator==(const Address& rhs) const;
        bool operator!=(const Address& rhs) const;
    };

    class IPAddress : public Address{
    public:
        using ptr = std::shared_ptr<IPAddress>;

        static IPAddress::ptr Create(const char* address, uint32_t port = 0);

        virtual uint32_t getPort() const = 0;
        virtual void setPort(uint32_t v) = 0;

        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
        virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;
    };

    class IPv4Address: public IPAddress{
    public:
        using ptr = std::shared_ptr<IPv4Address>;
        IPv4Address();
        IPv4Address(const sockaddr_in& address);
        IPv4Address(uint32_t address, uint32_t port);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;

        uint32_t getPort() const override;
        void setPort(uint32_t v) override;

        std::ostream& insert(std::ostream& os) const override;

        static IPv4Address::ptr Create(const char* address, uint32_t port = 0);

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    
    private:
        sockaddr_in m_addr;
    };

    class IPv6Address: public IPAddress{
        using ptr = std::shared_ptr<IPv6Address>;
        IPv6Address();
        IPv6Address(const sockaddr_in6& address);
        IPv6Address(const uint8_t address[16], uint32_t port);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;

        uint32_t getPort() const override;
        void setPort(uint32_t v) override;

        std::ostream& insert(std::ostream& os) const override;

        static IPv6Address::ptr Create(const char* address, uint32_t port = 0);

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
        IPAddress::ptr networkAddress(uint32_t prefix_len) override;
        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    private:
        sockaddr_in6 m_addr;
    };

    class UnixAddress: public Address{
    public:
        using ptr = std::shared_ptr<UnixAddress>;
        UnixAddress();
        UnixAddress(const std::string& path);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;

    private:
        struct sockaddr_un m_addr;
        socklen_t m_length;
    };

    class UnknowAddress: public Address{
    public:
        using ptr = std::shared_ptr<UnknowAddress>;
        UnknowAddress();
        UnknowAddress(int family);
        UnknowAddress(const sockaddr& addr);

        const sockaddr* getAddr() const override;
        socklen_t getAddrLen() const override;
        std::ostream& insert(std::ostream& os) const override;
    private:
        sockaddr m_addr;
    };
}