/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/09 10:59:39 by abablil           #+#    #+#             */
/*   Updated: 2025/01/09 18:29:25 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <map>

#define METHOD_GET "GET"
#define METHOD_POST "POST"
#define METHOD_DELETE "DELETE"
#define BOUNDARY_PREFIX "boundary="
#define CONTENT_LENGTH_PREFIX "Content-Length: "
#define HOST_PREFIX "Host: "

class Request
{
private:
	int port;
	int content_length;
	std::string ip;
	std::string path;
	std::string method;
	std::string body;
	std::string boundary;
	std::map<std::string, std::string> headers;

	void handleFirstLine(std::istringstream &requestStream);
	void clear();
public:	
	void parse(const std::string &request);
	const std::string &getBody() const;
	const std::string &getBoundary() const;
	const std::string &getHeaders() const;
	const std::string &getContentLength() const;
};
