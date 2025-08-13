#include "tcp_server.h"
#include "config.h"
#include "log.h"

namespace agent{

    static Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    static ConfigVar<uint64_t>::ptr g_tcp_read_timeout = Config::Lookup(
        "tcp_server.read_timeout", (uint64_t)(5000),"tcp server read timeout");
        
    TcpServer::TcpServer(IOManager* worker, IOManager* accept_worker)
    :m_worker(worker)
    ,m_acceptWorker(accept_worker)
    ,m_readTimeout(g_tcp_read_timeout -> getValue())
    ,m_name("agent/1.0.0")
    ,m_isStop(true){}

    TcpServer::~TcpServer(){
        for(auto& i : m_socks){
            i -> close();
        }
        m_socks.clear();
    }

    bool TcpServer::bind(Address::ptr addr){
        std::vector<Address::ptr> addrs;
        std::vector<Address::ptr> e_addrs;
        addrs.push_back(addr);

        return bind(addrs, e_addrs);
    }

    bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& e_addrs){
        for(auto& addr : addrs){
            Socket::ptr sock = Socket::CreateTCP(addr);
            if(!sock -> bind(addr)){
                AGENT_LOG_ERROR(g_logger) << "bind fail errno=" << errno << " errstr=" << strerror(errno)
                                          << " addr=[" << addr -> toString() << "]";
                e_addrs.push_back(addr);
                continue;
            }
            if(!sock -> listen()){
                AGENT_LOG_ERROR(g_logger) << "listen fail errno=" << errno << " strerror=" << strerror(errno)
                                          << " addr=[" << addr -> toString() << "]";
                e_addrs.push_back(addr);
                continue; 
            }
            m_socks.push_back(sock);
        }

        if(!e_addrs.empty()){
            m_socks.clear();
            return false;
        }

        for(auto& i : m_socks){
            AGENT_LOG_INFO(g_logger) << "server bind success: " << *i;
        }

        return true;
    }

    void TcpServer::startAccept(Socket::ptr sock) {
<<<<<<< HEAD
        AGENT_LOG_DEBUG(g_logger) << "[Start accept]";
        while(!m_isStop){
            Socket::ptr client = sock -> accept();
            //AGENT_LOG_WARN(g_logger) << "[Accept Socket] " << client -> toString();
            if(client){
=======
        AGENT_LOG_DEBUG(g_logger) << "[Accept] start accept coroutine.";
        while(!m_isStop){
            Socket::ptr client = sock -> accept();
            if(client){
                // AGENT_LOG_DEBUG(g_logger) << "[Accept] accept new client: " << client -> toString();
>>>>>>> f0ef15c (rebuild repository after corruption)
                client -> setRecvTimeout(m_readTimeout);
                m_worker -> schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
            }else{
                AGENT_LOG_ERROR(g_logger) << "accept errno=" << errno << " strerror=" << strerror(errno);
            }
        }
    }

    bool TcpServer::start(){
        if(!m_isStop){
            return true;
        }
        m_isStop = false;
        for(auto& sock: m_socks){
            m_acceptWorker -> schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
        }
        return true;
    }

    void TcpServer::stop(){
        m_isStop = false;
        auto self = shared_from_this();
        m_acceptWorker -> schedule([this, self](){
            for(auto & sock: m_socks){
                sock -> cancelAll();
                sock -> close();
            } 
            m_socks.clear();
        });
    }


    void TcpServer::handleClient(Socket::ptr client){
<<<<<<< HEAD
        AGENT_LOG_INFO(g_logger) << "handleClient: " << *client;
=======
        AGENT_LOG_INFO(g_logger) << "[handleClient] " << *client;
>>>>>>> f0ef15c (rebuild repository after corruption)
    }
}
