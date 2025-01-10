#include "Utils.hpp"

bool Utils::fileExists(const std::string &filePath)
{
  std::ifstream file(filePath.c_str());
  return file.is_open();
}

std::string	Utils::getCurrentTime()
{

  std::time_t current_time = std::time(0);
  struct tm *local_time = std::localtime(&current_time);
  char time_string[100];
  std::strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", local_time);

  return std::string(time_string);
}

// int	Utils::readFile(char* toFill, const std::string &filePath) {
	
// }
