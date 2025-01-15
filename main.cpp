#include "./Server/TcpServer.hpp"

int main()
{

	while (true)
	{
		try
		{
			TcpServer tcpserver;
			tcpserver.initializeServer(PORT);
			int result = tcpserver.handleIncomingConnections();
			if (result == 1)
			{
				std::cerr << "Restarting server due to critical error..." << std::endl;
				continue;
			}
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
		}
		break;
	}
	return 0;
}
