#include "./Server/WebServ.hpp"

int main(int ac, char **args)
{
	signal(SIGPIPE, SIG_IGN);
	if (ac > 2)
	{
		std::cout << "Usage : ./webserv (optional)config_file" << std::endl;
		return 1;
	}

	try
	{
		Config config(args[1] ? args[1] : "./Configs/webserv.conf");
		WebServ webserv(&config);

		webserv.initServers();
		if (webserv.getListeners().empty())
		{
			std::cerr << "No server created" << std::endl;
			return 1;
		}

		std::cout << GREEN << " _       ____________ _____ __________ _    __" << std::endl;
		std::cout << GREEN << "| |     / / ____/ __ ) ___// ____/ __ \\ |  / /" << std::endl;
		std::cout << GREEN << "| | /| / / __/ / __  \\__ \\/ __/ / /_/ / | / /" << std::endl;
		std::cout << GREEN << "| |/ |/ / /___/ /_/ /__/ / /___/ _, _/| |/ /" << std::endl;
		std::cout << GREEN << "|__/|__/_____/_____/____/_____/_/ |_| |___/" << std::endl << RESET << std::endl;


		webserv.handleServersIncomingConnections();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}