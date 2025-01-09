#include "Global.hpp"

int main()
{

	try
	{
		TcpServer tcpserver(8080, true);

		tcpserver.handleIncomingConnection();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
