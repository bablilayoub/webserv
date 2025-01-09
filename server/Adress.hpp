#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080

class Address {
	private:
		struct sockaddr_in serverAddress;
		int addrlen;
    
	public:
		Address() {
			memset(&this->serverAddress, 0, sizeof(this->serverAddress));
			this->serverAddress.sin_family = AF_INET;
			this->serverAddress.sin_port = htons(PORT);
			this->serverAddress.sin_addr.s_addr = INADDR_ANY;
		}

	struct sockaddr_in& getAddress()  {
		return serverAddress;
	}
	int getSizeOfServerAddress() const {
		return sizeof(serverAddress);
	}
	void print() {
		std::cout << &serverAddress << std::endl;
	}
};

