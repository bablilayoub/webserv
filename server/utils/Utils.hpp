#pragma once

#include <filesystem>
#include <ctime>
#include <iostream>

class Utils {
  public:
    char *getCurrentTime();
    bool fileExists(const std::string &filePath);
};