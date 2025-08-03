#include "iomanager.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/eventfd.h>
#include <string>

#include "macro.h"
#include "log.h"

namespace agent{

    static agent::Logger::ptr g_logger = AGENT_LOG_BY_NAME("system");

    IOManager::IOManager(size_t threads, bool use_caller, const std::string& name, bool idleFlag)
    :Scheduler(threads, use_caller, name, idleFlag){
        m_epfd = epoll_create(1);
        m_eventfd = eventfd(0, EFD_NONBLOCK);
        AGENT_ASSERT(m_epfd > 0);

        int rt = pipe(m_tickleFds);
        AGENT_ASSERT(rt == 0);

        epoll_event ep_event;
        memset(&ep_event, 0, sizeof(epoll_event));
        ep_event.events = EPOLLIN | EPOLLET;
        ep_event.data.fd =  m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        AGENT_ASSERT(rt == 0);

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &ep_event);
        AGENT_ASSERT(rt == 0);

        epoll_event evfd_event = {0};
        evfd_event.events = EPOLLIN;
        evfd_event.data.fd = m_eventfd;

        epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_eventfd, &evfd_event);

        contextResize(32);
        start();
    }
    IOManager::~IOManager()
    {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);
        close(m_eventfd);

        for(size_t i = 0; i < m_fdContexts.size(); ++ i)
        {
            if(m_fdContexts[i]){
                delete m_fdContexts[i];
            }
        }
    }


    int IOManager::addEvent(int fd, EventType event, std::function<void()> cb){
        FdContext* fd_ctx = nullptr;

        RWMutexType::ReadLock lock(m_rwmutex);

        if((int)m_fdContexts.size() > fd){
            lock.unlock();
            fd_ctx = m_fdContexts[fd];
        }else{
            lock.unlock();
            RWMutexType::WriteLock lock2(m_rwmutex);
            contextResize(fd * 1.5);
            fd_ctx = m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock3(fd_ctx -> mutex);

        if(fd_ctx -> event & event){
            AGENT_LOG_ERROR(g_logger) << "addEvent assert fd = " << fd 
                                    << " event = " << event 
                                    << " fd_ctx.event = " << fd_ctx -> event;
            AGENT_ASSERT(!(fd_ctx -> event & event));
        }
        int op = fd_ctx -> event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

        epoll_event ep_event;
        ep_event.events = EPOLLET | fd_ctx->event | event;
        ep_event.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &ep_event);
        if(-1 == rt){
            AGENT_LOG_ERROR(g_logger) << "epoll_ctl (" << m_epfd << ", "
                << op << ", " << fd << ", " << ep_event.events << ") :"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return -1;
        }
        ++ m_pendingEventCount;
        fd_ctx -> event = (EventType)(fd_ctx -> event | event);
        FdContext::EventContext& event_ctx = fd_ctx -> getContext(event);
        AGENT_ASSERT(!event_ctx.scheduler
                        && !event_ctx.coroutine
                        && !event_ctx.cb);

        event_ctx.scheduler = Scheduler::GetThis();
        if(cb){
            event_ctx.cb.swap(cb);
        }else{
            event_ctx.coroutine = Coroutine::GetThis();
            AGENT_ASSERT(event_ctx.coroutine -> getState() == Coroutine::State::EXEC);
        }
        AGENT_LOG_DEBUG(g_logger) << "[Add event] " << Utils::print_epoll_events(fd_ctx -> event);
        return 0;
    }

    bool IOManager::delEvent(int fd, EventType event){
        RWMutexType::ReadLock lock(m_rwmutex);
        if((int)m_fdContexts.size() < fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock(fd_ctx -> mutex);
        if(!(fd_ctx -> event & event)){
            return false;
        }
        EventType new_event = (EventType)(fd_ctx -> event & ~event);
        int op = new_event ? EPOLL_CTL_MOD: EPOLL_CTL_DEL;

        epoll_event ep_event;
        ep_event.events = EPOLLET | new_event;
        ep_event.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &ep_event);
        if(-1 == rt){
            AGENT_LOG_ERROR(g_logger) << "epoll_ctl (" << m_epfd << ", "
                << op << ", " << fd << ", " << ep_event.events << ") :"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        -- m_pendingEventCount;
        fd_ctx -> event = new_event;
        FdContext::EventContext& event_ctx = fd_ctx -> getContext(event);
        fd_ctx -> resetContext(event_ctx);
        return true;
    }

    bool IOManager::cancelEvent(int fd, EventType event){
        AGENT_LOG_INFO(g_logger) << "[Cancel Event] Cancel fd=" << fd << " event="  << Utils::print_epoll_events(event);
        RWMutexType::ReadLock lock(m_rwmutex);
        if((int)m_fdContexts.size() < fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock(fd_ctx -> mutex);
        if(!(fd_ctx -> event & event)){
            return false;
        }
        EventType new_event = (EventType)(fd_ctx -> event & ~event);
        int op = new_event ? EPOLL_CTL_MOD: EPOLL_CTL_DEL;

        epoll_event ep_event;
        ep_event.events = EPOLLET | new_event;
        ep_event.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &ep_event);
        if(-1 == rt){
            AGENT_LOG_ERROR(g_logger) << "epoll_ctl (" << m_epfd << ", "
                << op << ", " << fd << ", " << ep_event.events << ") :"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        AGENT_LOG_ERROR(g_logger) << "cancelEvent: " << fd_ctx -> fd << ", " << ep_event.events;
        fd_ctx -> triggerEvent(event);
        -- m_pendingEventCount;
        return true;
    }

    bool IOManager::cancelAll(int fd){  // 取消某句柄下的所有事件
        RWMutexType::ReadLock lock(m_rwmutex);
        if((int)m_fdContexts.size() < fd){
            return false;
        }
        FdContext* fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock(fd_ctx -> mutex);
        if(!(fd_ctx -> event)){
            return false;
        }

        int op = EPOLL_CTL_DEL;

        epoll_event ep_event;
        ep_event.events = 0;
        ep_event.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &ep_event);
        if(-1 == rt){
            AGENT_LOG_ERROR(g_logger) << "epoll_ctl (" << m_epfd << ", "
                << op << ", " << fd << ", " << ep_event.events << ") :"
                << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        if(fd_ctx -> event & READ){
            fd_ctx -> triggerEvent(READ);
            -- m_pendingEventCount;
        }
        if(fd_ctx -> event & WRITE){
            fd_ctx -> triggerEvent(WRITE);
            -- m_pendingEventCount;
        }
        
        AGENT_ASSERT(fd_ctx -> event == 0);
        return true;
    }
      

    IOManager* IOManager::GetThis(){ // 获取当前的io manager
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
    }  

    // void IOManager::tickle()
    // bool IOManager::stopping()
    // void IOManager::idle()

    void IOManager::contextResize(size_t size){
        m_fdContexts.resize(size);
        for(size_t i = 0; i < m_fdContexts.size(); ++ i){
            if(!m_fdContexts[i]){
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i] -> fd = i;
            }
        }
    }

    IOManager::FdContext::EventContext& IOManager::FdContext::getContext(EventType event){
        switch(event){
            case IOManager::READ:
                return read;
            case IOManager::WRITE:
                return write;
            default:
                AGENT_ASSERT_PARA(false, "getContext");
        }
    }

    bool IOManager::stopping(uint64_t& timeout){
        timeout = getNextTimer();
        return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::stopping();
    }

    bool IOManager::stopping(){
        uint64_t timeout = 0;
        return  stopping(timeout);
    }

    void IOManager::FdContext::resetContext(EventContext& ctx){
        ctx.scheduler = nullptr;
        ctx.coroutine.reset();
        ctx.cb = nullptr;
    }

    void IOManager::FdContext::triggerEvent(EventType ev){
        AGENT_LOG_DEBUG(g_logger) << "[Trigger Event] fd=" << fd
       << " trigger Event =" << ev
       << " fdContext events =" << event;
        // AGENT_ASSERT(event & ev);
        event = (EventType)(event & ~ev);

        EventContext& ev_ctx = getContext(ev);
        if(ev_ctx.cb){
            ev_ctx.scheduler -> schedule(ev_ctx.cb);
        }else{
            ev_ctx.scheduler -> schedule(ev_ctx.coroutine);
        }
        ev_ctx.scheduler = nullptr;
        return;
    }

    void IOManager::tickle(){
        uint64_t val = 0;
        size_t rt = write(m_eventfd, &val, sizeof(val));
        AGENT_ASSERT(rt == sizeof(val));
    }

    void IOManager::idle(){
        AGENT_LOG_DEBUG(g_logger) << "[IOMANAGER IDLE Coroutine] start";
        epoll_event* events = new epoll_event[64]();
        std::shared_ptr<epoll_event> shared_events(events, [&events](epoll_event* ptr){
            delete[] events;
        });

        while(true){
            uint64_t next_timeout = 0;
            AGENT_LOG_DEBUG(g_logger) << "[IOMANAGER IDLE Coroutine] Begin loop epoll_wait";
            if(stopping(next_timeout)){
                AGENT_LOG_INFO(g_logger) << "name = " << m_name << " idle stopping exit";
                break;
            }
            
            int rt = 0;
            do{
                static const int MAX_TIMEOUT = 5000;
                if(next_timeout != ~0ull){
                    next_timeout = (int)next_timeout > MAX_TIMEOUT? MAX_TIMEOUT: next_timeout; 
                }else{
                    next_timeout = MAX_TIMEOUT;
                }
                rt = epoll_wait(m_epfd, events, 64, (int)next_timeout);
                
                if(rt < 0 && errno == EINTR){

                }else{
                    break;
                }
            }while(true);

            std::vector<std::function<void()>> cbs;
            listExpiredCb(cbs);
            if(!cbs.empty()){
                schedule(cbs.begin(), cbs.end());
                cbs.clear();
            }

            for(int i = 0; i < rt; ++ i){
                epoll_event& event = events[i];
                if(event.data.fd == m_eventfd){
                    uint64_t val = 0;
                    while(read(m_eventfd, &val, sizeof(val)));
                    continue;
                }

                FdContext* fd_ctx = (FdContext*)event.data.ptr;
                AGENT_LOG_DEBUG(g_logger) << "[Thread "<< Utils::getThreadId() << "] " << fd_ctx -> fd << ", Tragger events: [" << Utils::print_epoll_events(event.events) << "]";
                FdContext::MutexType::Lock lock(fd_ctx -> mutex);
                if(event.events & (EPOLLERR | EPOLLHUP)){
                    event.events &= EPOLLIN | EPOLLOUT;
                }

                int real_events = NONE;
                if(event.events & EPOLLIN){
                    real_events |= READ;
                }
                if(event.events & EPOLLOUT){
                    real_events |= WRITE;
                }

                if((fd_ctx -> event & real_events) == NONE){
                    continue;
                }
                // AGENT_LOG_ERROR(g_logger) << fd_ctx -> fd << ", events: [" << Utils::print_epoll_events(event.events) << "]";
                int left_events = (fd_ctx -> event & real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                AGENT_LOG_DEBUG(g_logger) << "[Thread  "<< Utils::getThreadId() << "] " <<  fd_ctx -> fd << ", Left events: [" << Utils::print_epoll_events(event.events) << "]";

                int rt2 = epoll_ctl(m_epfd, op, fd_ctx -> fd, &event);
                if(rt2){
                    AGENT_LOG_ERROR(g_logger) << "epoll_ctl (" << m_epfd << ", "
                        << op << ", " << fd_ctx -> fd << ", " << event.events << ") :"
                        << rt << " (" << errno << ") (" << strerror(errno) << ")";
                }

                if(real_events & READ){
                    fd_ctx -> triggerEvent(READ);
                    -- m_pendingEventCount;
                }
                if(real_events & WRITE){
                    fd_ctx -> triggerEvent(WRITE);
                    -- m_pendingEventCount;
                }
            }

            Coroutine::ptr cur = Coroutine::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();

            raw_ptr -> swapOut();
        }
        AGENT_LOG_DEBUG(g_logger) << "[IOMANAGER IDLE Coroutine] end";
    }

    void IOManager::onTimerInsertedAtFront(){
        tickle();
    }
}