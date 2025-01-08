#include "Adress.hpp"

int main()
{

	int sockfd, new_socket;

	Address address;
	int addrlen = address.getSizeOfServerAddress();
	struct sockaddr_in& serverAddrRef = address.getAddress();

  	sockfd = socket(AF_INET, SOCK_STREAM, 0);
  	if (sockfd == -1) {
		std::cout << "Socket creation failed" << std::endl;
		return -1;
	}
  
    // std::cout <<  "=> 1       " << serverAddrRef.sin_family << std::endl;
    // std::cout <<  "=> 2       " << serverAddrRef.sin_port << std::endl;
    // std::cout <<  "=> 3       " << serverAddrRef.sin_addr.s_addr << std::endl;
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(sockfd, (struct sockaddr *)&serverAddrRef, sizeof(serverAddrRef)) == -1) {
		std::cout << "bind failed" << std::endl;
		close(sockfd);
		return -1;
	}
    if (listen(sockfd, 5) == -1) {
		std::cout << "Listening to server failed" << std::endl;
		close(sockfd);
		return -1;
	}
	std::cout << "Server is listening on port " << PORT << "..." << std::endl;
	while(true) {
	new_socket = accept(sockfd, (struct sockaddr *)&serverAddrRef, (socklen_t*)&addrlen);
	     if (new_socket == -1) {
	        std::cerr << "Accept failed" << std::endl;
	        close(sockfd);
	        return -1;
	    }	
	    std::cout << "New client connected!" << std::endl;	
	    char buffer[1024] = {0};
	    read(sockfd, buffer, sizeof(buffer));
	    std::cout << "Received message: " << buffer << std::endl;	
	    const char* message = "Hello from server!";
	    send(new_socket, message, strlen(message), 0);	
	     close(new_socket);
	}	
	close(sockfd);
	return 0;
}

