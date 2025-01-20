/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 09:07:41 by abablil           #+#    #+#             */
/*   Updated: 2025/01/20 11:14:09 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Global.hpp"

struct Location
{
    bool autoindex;
    std::string redirect;
    std::string upload_dir;
    std::string root_folder;
    std::string index;
    std::vector<std::string> accepted_methods;
};

struct Server
{
    int cgi_timeout;
    std::string host;
    std::string root_folder;
    std::vector<int> ports;
    std::string limit_client_body_size;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_pages;
    std::map<std::string, Location> locations;
};

class Config
{
private:
    std::ifstream configFile;

    int lineNumber;
    Server currentServer;
    Location currentLocation;
    std::string locationPath;
    std::stack<std::string> blockStack;

    int parseInt(const std::string &value);

    bool isValidDirectory(const std::string &path);
    
    std::string trimTrailingSlash(const std::string &path);

    void parseKeyValue(const std::string &line, std::string &key, std::string &value);
    void processServerBlock();
    void processLocationBlock(const std::string &line);
    void processClosingBrace();
    void handleKeyValue(const std::string &line);
    void trimWhitespace(std::string &line);
    void initStatusCodes();
    void initMimeTypes();
    bool isValidIpv4(const std::string &ip);

public:
    std::vector<Server> servers;
    std::map<std::string, std::string> mimeTypes;
	std::map<int, std::string> statusCodes;
    
    Config(const std::string &filePath);
    ~Config();
};
