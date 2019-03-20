#include "network.h"
#include "socket.h"

namespace eznet {

    EZSocket::EZSocket(Network *nw, const char *ip, int port)
        : network(nw),
          host_ip(ip),
          host_port(port),
          sock(0),
          sock_handle(nullptr),
          enable(false) {}

    EZSocket::~EZSocket() {
        // close socket
        this->network = nullptr;
        this->sock_handle = nullptr;
    }

    void EZSocket::send(const char *buf, size_t sz) const {
        this->network->send(*this, buf, sz);
    }

    void EZSocket::recv(const char *buf, size_t sz) const {
        this->sock_handle(buf, sz);
    }

    void EZSocket::set_handle(sockhandle_t func) {
        this->sock_handle = func;
    }
}