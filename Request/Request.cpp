/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 14:29:17 by abablil           #+#    #+#             */
/*   Updated: 2025/01/09 16:25:24 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

void Request::trim(std::string &line, std::string niddle)
{
	for (size_t i = 0; i < niddle.size(); i++)
	{
		size_t pos = line.find(niddle[i]);
		while (pos != std::string::npos)
		{
			std::cout << "Here" << std::endl;
			line = line.substr(pos + 1, line.size());
			pos = line.find(niddle[i]);
		}
	}
}

void Request::handleFirstLine(std::istringstream &requestStream)
{
	std::string line;
	size_t lastPos = 0;
	std::getline(requestStream, line);

	// get method
	lastPos = line.find(' ');
	this->method = line.substr(0, lastPos);
	line = line.substr(lastPos + 1, line.size());

	// get path
	lastPos = line.find(' ');
	this->path = line.substr(0, lastPos);
	line = line.substr(lastPos + 1, line.size());
}

void Request::parse(const std::string &request)
{
	std::istringstream requestStream(request);

	this->handleFirstLine(requestStream);

	std::string line;
	while (std::getline(requestStream, line))
	{
		std::size_t boundaryPos = line.find(BOUNDARY_PREFIX);
		std::size_t contentLengthPos = line.find(CONTENT_LENGTH_PREFIX);
		std::size_t hostPrefixPos = line.find(HOST_PREFIX);
		
		if (boundaryPos != std::string::npos)
		{
			boundaryPos += std::string(BOUNDARY_PREFIX).length();
			this->boundary = line.substr(boundaryPos, line.size() - boundaryPos);
		}
		if (contentLengthPos != std::string::npos)
		{
			contentLengthPos += std::string(CONTENT_LENGTH_PREFIX).length();
			this->content_length = std::atof(line.substr(contentLengthPos).c_str());
		}
		if (hostPrefixPos != std::string::npos)
		{
			hostPrefixPos += std::string(HOST_PREFIX).length() + 1;
			line = line.substr(hostPrefixPos, line.size() - hostPrefixPos);
			this->ip = line.substr(0, line.find(':'));
			std::cout << "|" << line << "|" << std::endl;
			this->port = std::atof(line.substr(line.find(':')).c_str());
		}
	}

	std::cout << "|" << this->method << "|" << std::endl;
	std::cout << "|" << this->path << "|" << std::endl;
	std::cout << "|" << this->boundary << "|" << std::endl;
	std::cout << "|" << this->content_length << "|" << std::endl;
	std::cout << "|" << this->ip << "|" << std::endl;
	std::cout << "|" << this->port << "|" << std::endl;
	
	std::cout << request << std::endl;
}
