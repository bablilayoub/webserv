#include "./Server/TcpServer.hpp"
#include "./Client/Config.hpp"

void launchServers(std::vector<TcpServer> &tcpservers)
{
	size_t i = -1;
	while (++i < tcpservers.size())
	{
		int result = tcpservers[i].handleIncomingConnections();
		if (result == 1)
		{
			std::cerr << "Restarting server " << i + 1 << " due to critical error..." << std::endl;
			continue;
		}
		if (i == tcpservers.size() - 1)
			i = -1;
	}
}

void initServers(std::vector<TcpServer> &tcpservers, Config *config)
{
	uint16_t serversSize = config->servers.size();
	for (size_t i = 0; i < serversSize; i++)
	{
		tcpservers.push_back(TcpServer(config));
		tcpservers[i].initializeServer(config->servers[i].listen_port);
		tcpservers[i].AddClientSocket(tcpservers[i].getListner(), POLLIN);
	}
}

int main(int ac, char **args)
{
	if (ac != 2)
	{
		std::cout << "Usage : ./webserv [configuration file]" << std::endl;
		return 1;
	}

	Config config(args[1]);
	std::vector<TcpServer> tcpservers;
	initServers(tcpservers, &config);
	launchServers(tcpservers);
}