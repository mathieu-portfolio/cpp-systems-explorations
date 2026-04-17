#include "arena.hpp"
#include <iostream>

int main()
{
  Arena arena(32, 8);

  std::cout << "before\n";
  (void)arena.allocate(8, 0);
  std::cout << "after\n";

  return 0;
}