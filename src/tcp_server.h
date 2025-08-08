#pragma once

#include <memory>
#include <functional>

#include "iomanager.h"
#include "address.h"
#include "socket.h"
#include "noncopyable.h"

namespace agent{
    class TcpServer: public std::enable_shared_from_this<TcpServer>, Noncopyable{
    public:
        using ptr = std::shared_ptr<TcpServer>;
        TcpServer(IOManager* worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());

        virtual ~TcpServer(){}
        virtual bool bind(Address::ptr addr);
        virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& e_addrs);
        virtual bool start();
        void bool stop();

        uint64_t getReadTimeout() const {return m_readTimeout;}
        std::string getName() const {return m_name;}
        void setReadTimeout(uint64_t v) {m_readTimeout = v;}
        void setName(const std::string name){m_name = name;}

        bool isStop() const {return m_isStop;}

    protected:
        virtual void handleClient(Socket::ptr client);
        virtual void startAccept(Socket::ptr sock);
    private:
        std::vector<Socket::ptr> m_socks;
        IOManager* m_worker;
        IOManager* m_acceptWorker;
        uint64_t m_readTimeout;
        std::string m_name;
        bool m_isStop;
    };
}