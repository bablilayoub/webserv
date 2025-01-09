#include "Utils.hpp"

bool Utils::fileExists(const std::string &filePath)
{
  return std::filesystem::exists(filePath);
}

char* Utils::getCurrentTime()
{
  std::time_t t = std::time(nullptr);
  std::tm *tm_info = std::gmtime(&t);

  char buffer[80];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

  std::cout << "Current date and time (GMT): " << buffer << std::endl;
  return buffer;
}