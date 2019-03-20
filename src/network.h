#pragma once

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

        void connet(EZSocket **sock, const char *ip, int port);
        void closesock(const EZSocket &sock);
        void send(const EZSocket &sock, const char *buf, size_t sz);

    private:
        EventLoopInterface *loop;
    };
}
