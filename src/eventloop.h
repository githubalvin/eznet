#pragma once

#include <WinSock2.h>

#ifdef WIN32
#define SOCKLEN int
#else
#define SOCKET int
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define SOCKLEN socklen_t
#endif

#define BUF_SIZE 1024

namespace eznet {
    class Network;

    class EventLoopInterface {
    public:
        EventLoopInterface() = default;
		virtual ~EventLoopInterface() {};

        virtual int initialize(Network *network) = 0;
        virtual int finalize() = 0;
        virtual int process(int wait) = 0;
        virtual int sendcmd(void* buf, size_t sz) = 0;
        virtual int processcmd(void* buf, size_t sz) = 0;
		virtual int listen(int port) = 0;
		virtual int socket() = 0;
		virtual void closesocket(int sock) = 0;
		virtual int forward(SOCKET &sock, char* buf, size_t sz) = 0;
    protected:
        Network* network;
    };

}