#include "fd_manager.h"

#include <sys/stat.h>
#include <fcntl.h>

#include "hook.h"

namespace agent{

    FdCtx::FdCtx(int fd)
    :m_isInit(false)
    ,m_isSocket(false)
    ,m_sysNonblock(false)
    ,m_userNonblock(false)
    ,m_isClosed(false)
    ,m_fd(fd)
    ,m_recvTimeout(-1)
    ,m_sendTimeout(-1){
        init();
    }

    FdCtx::~FdCtx(){}

    bool FdCtx::init(){
        if(m_isInit){
            return true;
        }
        m_recvTimeout = -1;
        m_sendTimeout = -1;

        struct stat fd_stat;
        if(-1 == fstat(m_fd, &fd_stat)){
            m_isInit = false;
            m_isSocket = false;
        }else{
            m_isInit = true;
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }

        if(m_isSocket){
            int flag = fcntl(m_fd, F_GETFL, 0);
            if(!(flag & O_NONBLOCK)){
                fcntl_f(m_fd, F_SETFL, flag |  O_NONBLOCK);
            }
        }

        m_userNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }

    void setTimeout(int type, uint64_t v){

    }

    uint64_t getTimeout(int type){

    }

    FdManager::FdManager(){
        m_datas.resize(64);
    }

    FdCtx::ptr get(int fd, bool autp_create = false){

    }

    void del(int fd){

    }
}