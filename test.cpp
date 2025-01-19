
#include <iostream>
void increment(int &i)
{
  while (i <= 10)
    i++;
}

int main(int ac, char **args)
{
  int i = 0;
  increment(i);
  std::cout << i << std::endl;
}