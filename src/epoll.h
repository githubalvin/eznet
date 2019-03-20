#pragma once

#ifndef WIN32
#include "eventloop.h"
#include <sys/socket.h>
#include <sys/epoll.h>

namespace eznet {
    class Network;
    
    class EventLoop : public EventLoopInterface {
        enum { EPOLL_SIZE = 50 };

    public:
        EventLoop() = default;
        ~EventLoop() {};

        int initialize(Network *network);
        int finalize();
        int listen(int port);
        int sendcmd(void* buf, size_t sz);
        int processcmd(void* buf, size_t sz);
        int process(int wait);
        int forward(SOCKET &sock, char* buf, size_t sz);
    
    private:
        SOCKET listen_sock;
        SOCKADDR_IN serv_addr;
        char buf[BUF_SIZE];
        int pipe_fds[2];

        epoll_event *ep_events;
        epoll_event event;
        int epfd;
    };
}
#endif