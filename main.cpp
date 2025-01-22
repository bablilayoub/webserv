#include "./Server/WebServ.hpp"

int main(int ac, char **args)
{
	if (ac != 2)
	{
		std::cout << "Usage : ./webserv [configuration file]" << std::endl;
		return 1;
	}

	while (true)
	{
		try
		{
			Config config(args[1]);
			WebServ webserv(&config);

			webserv.initServers();
			webserv.handleServersIncomingConnections();
		}
		catch (std::exception &e)
		{
			std::cout << "Error: " << e.what() << std::endl;
		}
	}
}