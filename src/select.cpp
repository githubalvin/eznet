#include <iostream>
#include "network.h"
#include "socket.h"
#include "select.h"

namespace eznet {

	static int pipe(int fildes[2]) {
		int tcp1, tcp2;
		SOCKADDR_IN name;
		memset(&name, 0, sizeof(name));
		name.sin_family = AF_INET;
		name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		int namelen = sizeof(name);
		tcp1 = tcp2 = -1;
		SOCKET tcp = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp == -1) {
			goto clean;
		}
		if (bind(tcp, (SOCKADDR*)&name, namelen) == -1) {
			goto clean;
		}
		if (listen(tcp, 5) == -1) {
			goto clean;
		}
		if (getsockname(tcp, (SOCKADDR*)&name, &namelen) == -1) {
			goto clean;
		}
		tcp1 = socket(AF_INET, SOCK_STREAM, 0);
		if (tcp1 == -1) {
			goto clean;
		}
		if (-1 == connect(tcp1, (SOCKADDR*)&name, namelen)) {
			goto clean;
		}
		tcp2 = accept(tcp, (SOCKADDR*)&name, &namelen);
		if (tcp2 == -1) {
			goto clean;
		}
		if (closesocket(tcp) == -1) {
			goto clean;
		}
		fildes[0] = tcp1;
		fildes[1] = tcp2;
		return 0;
	clean:
		if (tcp != -1) {
			closesocket(tcp);
		}
		if (tcp2 != -1) {
			closesocket(tcp2);
		}
		if (tcp1 != -1) {
			closesocket(tcp1);
		}
		return -1;
	}

	static void closepipe(int fildes[2]) {
		if (fildes[0] != -1)
			closesocket(fildes[0]);
		if (fildes[1] != -1)
			closesocket(fildes[1]);
	}
    
	EventLoop::~EventLoop() {
		this->network = nullptr;
	}

    int EventLoop::initialize(Network *nw) {
		this->network = nw;
        WSADATA wsa_data;
        if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
			return -1;
        }
		if (pipe(this->pipe_fds) != 0)
			return -2;

		FD_ZERO(&this->read_fds);
		FD_SET(this->pipe_fds[0], &this->read_fds);
        return 0;
    }

	int EventLoop::finalize() {
		closesocket(this->listen_sock);
		closepipe(this->pipe_fds);
		WSACleanup();
		return 0;
	}

    int EventLoop::listen(int port) {
        this->listen_sock = socket(PF_INET, SOCK_STREAM, 0);
        memset(&this->serv_addr, 0, sizeof(this->serv_addr));
        this->serv_addr.sin_family = AF_INET;
        this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        this->serv_addr.sin_port = htons(port);

        if(bind(this->listen_sock, (SOCKADDR*) &this->serv_addr, sizeof(this->serv_addr))==SOCKET_ERROR)
            return -1;
        
        if(::listen(this->listen_sock, 5)==SOCKET_ERROR)
            return -2;
        
        FD_SET(this->listen_sock, &this->read_fds);
        printf("listen: %d\n", port);
        return 0;
    }

    int EventLoop::process(int wait) {
        auto copy_reads = this->read_fds;
        auto copy_writes = this->write_fds;
        auto copy_excepts = this->except_fds;

        TIMEVAL timeout;
        timeout.tv_sec = wait;
        timeout.tv_usec = 0; 

        auto fds = select(0, &copy_reads, 0, 0, &timeout);
		if (fds == SOCKET_ERROR) {
			std::cout << WSAGetLastError() << std::endl;
			return -1;
		}
        
        for(int i = 0; i < read_fds.fd_count; ++i) {
            if(FD_ISSET(this->read_fds.fd_array[i], &copy_reads)) {
				if (this->read_fds.fd_array[i] == this->pipe_fds[0]) {
					auto str_len = recv(this->read_fds.fd_array[i], this->buf, BUF_SIZE - 1, 0);
					this->processcmd(buf, str_len);
				}
				else if (this->read_fds.fd_array[i] == this->listen_sock) {
					SOCKADDR_IN new_addr;
					SOCKLEN sz = sizeof(new_addr);
					SOCKET new_sock = accept(this->listen_sock, (SOCKADDR*)&new_addr, &sz);
					FD_SET(new_sock, &this->read_fds);
					std::cout << "accept new connection: " << inet_ntoa(new_addr.sin_addr) << std::endl;
				}
                else {
                    auto str_len = recv(this->read_fds.fd_array[i], this->buf, BUF_SIZE - 1, 0);
                    if(str_len == 0) {
                        FD_CLR(this->read_fds.fd_array[i], &this->read_fds);
                        closesocket(copy_reads.fd_array[i]);
                    }
                    else {
						forward(this->read_fds.fd_array[i], this->buf, str_len);
                    }
                }
            }
        }
		return 0;
    }

	int EventLoop::sendcmd(void* buf, size_t sz) {
		send(this->pipe_fds[1], (char*)buf, sz, 0);
		return 0;
	}

	int EventLoop::processcmd(void* buf, size_t sz) {
		auto cmd = (sock_cmd*)buf;
		switch (cmd->tp)
		{
		case sock_type::SOCKET_CREATE:
			SOCKADDR_IN serv_addr;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = inet_addr(cmd->ip);
			serv_addr.sin_port = htons(cmd->port);

			if (::connect(cmd->sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
				std::cout << "connet " << cmd->ip << ":" << cmd->port << " failed!" << std::endl;
				return -1;
			}

			FD_SET(cmd->sock, &this->read_fds);
			std::cout << "connet " << cmd->ip << ":" << cmd->port << " suc!" << std::endl;
			break;

		case sock_type::SOCKET_SEND:
			send(cmd->sock, cmd->buf, cmd->sz, 0);
			std::cout << "send " << ":" << cmd->buf << std::endl;
			break;

		case sock_type::SOCKET_CLOSE:
			FD_CLR(cmd->sock, &this->read_fds);
			closesocket(cmd->sock);
			std::cout << "closesock " << cmd->sock << std::endl;
			break;

		default:
			break;
		}
		return 0;
	}

    int EventLoop::forward(SOCKET &sock, char* buf, size_t sz) {
		std::cout << "receive " << buf << std::endl;
		return 0;
    }

}