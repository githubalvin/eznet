#ifndef WIN32
#include <iostream>
#include "network.h"
#include "socket.h"
#include "epoll.h"

namespace eznet {

	EventLoop::~EventLoop() {
        this->network = nullptr;
    }

    int EventLoop::initialize(Network *nw) {
         = nw;
        this->epfd = ethis->networkpoll_create(EPOLL_SIZE);
        this->ep_events = malloc(sizeof(epoll_event)*EPOLL_SIZE)
        if (pipe(this->pipe_fds) != 0)
			return -1;
        this->event.events = EPOLLIN;
        this->event.data.fd = this->pipe_fds[0];
        epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->pipe_fds[0], &this->event);
        return 0;
    }

	int EventLoop::finalize() {
        close(this->listen_sock);
        close(this->epfd);
		return 0;
	}

    int EventLoop::listen(int port) {
        this->listen_sock = socket(PF_INET, SOCK_STREAM, 0);
        memset(&this->serv_addr, 0, sizeof(this->serv_addr));
        this->serv_addr.sin_family = AF_INET;
        this->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        this->serv_addr.sin_port = htons(port);

        if(bind(this->listen_sock, (SOCKADDR*) &this->serv_addr, sizeof(this->serv_addr))==-1)
            return -1;
        
        if(::listen(this->listen_sock, 5)==-1)
            return -2;
        
        this->event.events = EPOLLIN;
        this->event.data.fd = this->listen_sock;
        epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->listen_sock, &this->event);
        printf("listen: %d\n", port);
        return 0;
    }

    int EventLoop::process(int wait) {
        auto event_cnt = epoll_wait(this->epfd, this->ep_events, EPOLL_SIZE, wait);
        if(event_cnt == -1) {
            return -1;
        }
        for(int i = 0; i < event_cnt; ++i) {
            if (this->ep_events[i].data.fd == this->pipe_fds[0]) {
					auto str_len = read(this->read_fds.fd_array[i], this->buf, BUF_SIZE - 1);
					this->processcmd(buf, str_len);
				}
            else if(this->ep_events[i].data.fd == this->listen_sock) {
                SOCKADDR_IN new_addr;
                SOCKLEN sz = sizeof(new_addr);
                SOCKET new_sock = accept(this->listen_sock, (SOCKADDR*)&new_addr, &sz);
                
                this->event.events = EPOLLIN;
                this->event.data.fd = new_sock;
                epoll_ctl(this->epfd, EPOLL_CTL_ADD, new_sock, &this->event);
                std::cout << "accept new connection: " << inet_ntoa(new_addr.sin_addr) << std::endl;
            }
            else {
                auto str_len = read(this->ep_events[i].data.fd, this->buf, BUF_SIZE - 1);
                if(str_len == 0) {
                    epoll_ctl(this->epfd, EPOLL_CTL_DEL, this->ep_events[i].data.fd, NULL);
                    close(this->ep_events[i].data.fd);
                }
                else {
                    forward(this->ep_events[i].data.fd, this->buf, str_len);
                }
            }
        }
		return 0;
    }

	int EventLoop::sendcmd(void* buf, size_t sz) {
		write(this->pipe_fds[1], (char*)buf, sz);
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

			if (::connect(cmd->sock, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == -1) {
				std::cout << "connet " << cmd->ip << ":" << cmd->port << " failed!" << std::endl;
				return -1;
			}

            this->event.events = EPOLLIN;
            this->event.data.fd = cmd->sock;
            epoll_ctl(this->epfd, EPOLL_CTL_ADD, cmd->sock, &this->event);

			std::cout << "connet " << cmd->ip << ":" << cmd->port << " suc!" << std::endl;
			break;

		case sock_type::SOCKET_SEND:
			write(cmd->sock, cmd->buf, cmd->sz);
			std::cout << "send " << ":" << cmd->buf << std::endl;
			break;

		case sock_type::SOCKET_CLOSE:
            epoll_ctl(this->epfd, EPOLL_CTL_DEL, cmd->sock, NULL);
            close(cmd->sock);
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
#endif // !WIN32