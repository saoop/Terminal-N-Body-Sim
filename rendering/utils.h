#pragma once
#include <iostream>
void clearFullTerminal() {
  std::cout << "\033[2J"; // clear entire screen once
  std::cout << "\033[H";  // move to top left once
  std::cout << "\033[s";  // save this as our origin
                          // clear the screen
}