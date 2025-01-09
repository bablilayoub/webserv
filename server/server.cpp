#include "Global.hpp"

int main()
{
	TcpServer tcpserver;

	if (tcpserver.socketCreationAndBinding() == -1)
		return -1;
	std::cout << "Server is listening on port " << PORT << "..." << std::endl;
	if (tcpserver.handleIncomingConnection() == -1)
		return -1;
	return 0;
}
