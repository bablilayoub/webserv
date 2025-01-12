/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abablil <abablil@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 09:07:41 by abablil           #+#    #+#             */
/*   Updated: 2025/01/12 17:55:16 by abablil          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <exception>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <stack>

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
    int listen_port;
    std::string limit_client_body_size;
    std::vector<std::string> server_names;
    std::map<std::string, Location> locations;
    std::map<int, std::string> error_pages;
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

    void parseKeyValue(const std::string &line, std::string &key, std::string &value);
    int parseInt(const std::string &value);
    void processServerBlock();
    void processLocationBlock(const std::string &line);
    void processClosingBrace();
    void handleKeyValue(const std::string &line);
    void trimWhitespace(std::string &line);

public:
    std::vector<Server> servers;

    Config(const std::string &filePath);
    ~Config();
};
