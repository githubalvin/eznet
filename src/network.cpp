#include "socket.h"
#ifdef WIN32
#include "select.h"
#else
#include "epoll.h"
#endif
#include "network.h"

namespace eznet {
    Network::Network()
        : loop(nullptr),
          sockets() {}

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
		for (auto &s : this->sockets)
			closesock(*s.second);
		//��Ҫ�ȴ������¼���������սᣬ�ȱ���ʵ��
        return this->loop->finalize();
    }

    int Network::process(int wait) {
        this->loop->process(wait);
        return 0;
    }


    void Network::find_socket(EZSocket **sock, int index) const {
		auto iter = this->sockets.find(index);
		if (iter != this->sockets.end())
			*sock = iter->second;
    }

    void Network::new_socket(EZSocket **sock, const char *ip, int port, int s) {
		if(0==s)
			s = this->loop->socket();
		auto ezsock = new EZSocket(this, ip, port);
		ezsock->set_sock(s);
		*sock = ezsock;
        this->sockets[ezsock->get_sock()] = ezsock;
    }

	void Network::add_socket(EZSocket *sock) {
		this->sockets[sock->get_sock()] = sock;
	}

	void Network::remove_socket(int index) {
		this->sockets.erase(index);
	}

    void Network::connet(EZSocket **sock, const char *ip, int port) {
        /*TODO: to solve multi-thread problem
        */
        new_socket(sock, ip, port, 0);
        auto s = *sock;
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