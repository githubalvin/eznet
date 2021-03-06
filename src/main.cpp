#include <iostream>
#include "network.h"
#include "socket.h"

using namespace eznet;
using namespace std;

int main(int argc, char *argv[]) {
	Network network;
	if (network.initialize(6001)!= 0) {
		printf("network initialize failed\n");
		network.finalize();
		return -1;
	};
	EZSocket *sock;
	network.connet(&sock, "127.0.0.1", 6001);
	char buf[] = "hello";

	int cnt = 5;
	while (cnt > 0) {
		--cnt;
		network.process(1);
		sock->send(buf, sizeof(buf));
		Sleep(1000);
		network.process(1);
	}
	network.closesock(*sock);
	network.process(1);
	network.finalize();
    return 0;
}