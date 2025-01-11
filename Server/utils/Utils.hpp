#pragma once

#include <fstream>
#include <ctime>
#include <iostream>

class Utils {
  public:
    std::string getCurrentTime();
    bool	fileExists(const std::string &filePath);
    int		readFile(char* toFill, const std::string &filePath); 
};