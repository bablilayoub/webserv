/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aitaouss <aitaouss@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 10:59:39 by abablil           #+#    #+#             */
/*   Updated: 2025/01/13 18:45:41 by aitaouss         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../FileUpload/FileUpload.hpp"

#include <iostream>
#include <sstream>
#include <map>

#define BOUNDARY_PREFIX "boundary="
#define CONTENT_LENGTH_PREFIX "Content-Length: "
#define HOST_PREFIX "Host: "

class Client
{
private:
	int clientFd;
	int port;
	int content_length;
	bool firstChunk;
	std::string ip;
	std::string path;
	std::string method;
	std::string body;
	std::string boundary;
	std::map<std::string, std::string> headers;
	
	void handleFirstLine(std::istringstream &requestStream);
	void clear();

public:
	Client();
	void setSocketFd(int fd);
	void parse(const std::string &request, std::map<int, FileUpload>& BodyMap);
	const std::string &getBody() const;
	const std::string &getBoundary() const;
	const std::map<std::string, std::string> &getHeaders() const;
	const int &getContentLength() const;
};
