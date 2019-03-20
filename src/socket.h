#pragma once

#include "eventloop.h"

namespace eznet {

    enum sock_type {
        SOCKET_CREATE = 1,
        SOCKET_CLOSE = 2,
        SOCKET_SEND = 3,
    };

    struct sock_cmd {
        sock_type tp;
        union
        {
            /*create
            */
            struct {
                SOCKET sock;
                const char* ip;
                int port;
            };
            /*close
            */
            struct {
                SOCKET sock;
            };
            /*send
            */
            struct{
                SOCKET sock;
                const char *buf;
                size_t sz;
            };
        };
    };

    class Network;

    class EZSocket {
    public:
        using sockhandle_t = void (*)(const char*, size_t);

        EZSocket(Network *network, const char *ip, int port);
        ~EZSocket();

        inline const SOCKET get_sock() const { return this->sock; }
        inline const char* ip() const { return this->host_ip; }
        inline int port() const { return this->host_port; } 

        void send(const char *buf, size_t sz) const;
        void recv(const char *buf, size_t sz) const;
        void set_handle(sockhandle_t handle);

    private:
        Network *network;
        const char *host_ip;
        int host_port;
        SOCKET sock;
        sockhandle_t sock_handle;
    };
}