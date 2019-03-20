#pragma once

#include <map>

namespace eznet {
    class EZSocket;
    class EventLoopInterface;

    class Network {
    public:
        Network();
        ~Network();

        int initialize(int port);
        int finalize();
        int process(int wait);

        void find_socket(EZSocket **sock, int index) const;
        void new_socket(EZSocket **sock, const char *ip, int port, int s);
		void add_socket(EZSocket *sock);
		void remove_socket(int index);

        void connet(EZSocket **sock, const char *ip, int port);
        void closesock(const EZSocket &sock);
        void send(const EZSocket &sock, const char *buf, size_t sz);

    private:
        EventLoopInterface *loop;
        std::map<int, EZSocket*> sockets;
    };
}
