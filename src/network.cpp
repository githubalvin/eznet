#include "socket.h"
#ifdef WIN32
#include "select.h"
#else
#include "epoll.h"
#endif
#include "network.h"

namespace eznet {
    Network::Network()
        : loop(nullptr) {}

    Network::~Network() {
        delete this->loop;
        this->loop = nullptr;
    }

    int Network::initialize(int port) {
        this->loop = new EventLoop;
        if(this->loop->initialize(this)!=0)
            return -1;
        return this->loop->listen(port);
    }

    int Network::finalize() {
        return this->loop->finalize();
    }

    int Network::process(int wait) {
        this->loop->process(wait);
        return 0;
    }

    void Network::connet(EZSocket **sock, const char *ip, int port) {
        /*TODO: to solve multi-thread problem
        */
        auto s = new EZSocket(this, ip, port);
        *sock = s;

        sock_cmd cmd;
        cmd.tp = SOCKET_CREATE;
        cmd.sock = s->get_sock();
        cmd.ip = ip;
        cmd.port = port;
        this->loop->sendcmd((void*)&cmd, sizeof(cmd));
    }

    void Network::closesock(const EZSocket &sock) {
        sock_cmd cmd;
        cmd.tp = SOCKET_CLOSE;
        cmd.sock = sock.get_sock();
        this->loop->sendcmd((void*)&cmd, sizeof(cmd));
    }

    void Network::send(const EZSocket &sock, const char *buf, size_t sz) {
        sock_cmd cmd;
        cmd.tp = SOCKET_SEND;
        cmd.sock = sock.get_sock();
        cmd.buf = buf;
        cmd.sz = sz;
        this->loop->sendcmd((void*)&cmd, sizeof(cmd));
    }
}