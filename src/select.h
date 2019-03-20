#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eventloop.h"


namespace eznet {
    class Network;

    class EventLoop : public EventLoopInterface {
    public:
        EventLoop() = default;
        ~EventLoop();

        int initialize(Network *network);
        int finalize();
        int process(int wait);
        int sendcmd(void* buf, size_t sz);
        int processcmd(void* buf, size_t sz);
        int listen(int port);
		int socket();
		void closesocket(int sock);
        int forward(SOCKET &sock, char* buf, size_t sz);

    private:
        SOCKET listen_sock;
        SOCKADDR_IN serv_addr;
        char buf[BUF_SIZE];
        int pipe_fds[2];

        fd_set read_fds;
        fd_set write_fds;
        fd_set except_fds;
    };

}




