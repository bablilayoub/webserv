/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 09:29:12 by abablil           #+#    #+#             */
/*   Updated: 2025/01/12 17:46:04 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

int main()
{
	try
	{
		Config config("webserv.config");

		for (const auto &server : config.servers)
		{
			std::cout << "Listen Port: '" << server.listen_port << "'" << std::endl;
			std::cout << "Server Names: " << std::endl;
			for (const auto &server_name : server.server_names)
				std::cout << "Name: '" << server_name << "'" << std::endl;
				std::cout << "Limit Client Body Size: '" << server.limit_client_body_size << "'" << std::endl;

			for (const auto &location : server.locations)
			{
				std::cout << "Location: '" << location.first << "'" << std::endl;
				std::cout << "Upload Dir: '" << location.second.upload_dir << "'" << std::endl;
				std::cout << "Redirect: '" << (!location.second.redirect.empty() ? location.second.redirect : "None") << "'" << std::endl;
				std::cout << "AutoIndex: '" << (location.second.autoindex ? "on" : "false") << "'" << std::endl;
				std::cout << "Accepted Methods:" << std::endl;
				for (const auto &method : location.second.accepted_methods)
					std::cout << "Method: '" << method << "'" << std::endl;
			}

			for (const auto &error_page : server.error_pages)
				std::cout << "Error Page '" << error_page.first << "'" << ": " << "'" << error_page.second << "'" << std::endl;

			std::cout << std::endl;
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}