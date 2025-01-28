#include "./Server/WebServ.hpp"

int main(int ac, char **args)
{
	signal(SIGPIPE, SIG_IGN);
	if (ac != 2)
	{
		std::cout << "Usage : ./webserv [configuration file]" << std::endl;
		return 1;
	}

	try
	{
		Config config(args[1]);
		WebServ webserv(&config);

		webserv.initServers();
		if (webserv.getListeners().empty())
		{
			std::cerr << "No server created" << std::endl;
			return 1;
		}

		// std::cout << GREEN << "|--------------------------------------------|" << std::endl;
		// std::cout << "|                                            |" << std::endl;
		// std::cout << "|           WEBSERV IS RUNNING               |" << std::endl;
		// std::cout << "|                                            |" << std::endl;
		// std::cout << "|--------------------------------------------|" << RESET << std::endl << std::endl;

		webserv.handleServersIncomingConnections();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}