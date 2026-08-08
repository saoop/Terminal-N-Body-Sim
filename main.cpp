#include "app.h"
#include "common/input_controller.h"
#include "parser/parser.h"
#include "states/states.h"
#include <cmath>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
RawMode *raw_mode = nullptr; // RAII for raw mode

void signalHandler(int signal) {
  if (raw_mode) {
    disableRawMode(raw_mode->original);
    std::cout << "\033[?25h";     // show cursor
    std::cout << "\033[2J\033[H"; // optional: clear screen
  }
  std::exit(signal);
}

int main() {
  raw_mode = new RawMode(); // RAII for raw mode

  std::signal(SIGINT, signalHandler);  // for Ctrl+C
  std::signal(SIGTERM, signalHandler); // for termination

  App app{};
  app.setState(std::make_unique<MenuState>(app));
  app.run();

  return 0;
}