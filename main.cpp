#include "Server/Global.hpp"
#include "Client/Config.hpp"

int main(int ac, char **args)
{
	if (ac != 2)
	{
		std::cout << "Usage : ./webserv [config file path]" << std::endl;
		return 1;
	}

	while (true)
	{
		try
		{
			Config config(args[1]);
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
