#ifndef INPUT_CONTROLLER_H
#define INPUT_CONTROLLER_H

#include "commands.h"
#include <iostream>
#include <memory>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>

enum class Key { UP, DOWN, RIGHT, LEFT, SPACE, ZOOM_IN, ZOOM_OUT, OTHER, NONE };

void enableRawMode(termios &original) {
  termios raw;
  tcgetattr(STDIN_FILENO, &original); // save original
  raw = original;
  raw.c_lflag &= ~(ICANON | ECHO); // disable buffering and echo
  raw.c_cc[VMIN] = 0;              // non-blocking
  raw.c_cc[VTIME] = 0;             // no timeout
  tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disableRawMode(termios &original) {
  tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

struct RawMode {
  termios original;
  RawMode() {
    enableRawMode(original);
    std::cout << "\033[?25l"; // hide cursor
  }
  ~RawMode() {
    disableRawMode(original);
    std::cout << "\033[?25h"; // show cursor
  }
};

Key readKey() {
  char buf[3];
  int n = read(STDIN_FILENO, buf, 3);

  if (n <= 0)
    return Key::NONE;

  // Check the first one in the buffer
  switch (buf[0]) {
  case ' ':
    return Key::SPACE;
  case '-':
    return Key::ZOOM_OUT; // Using just + and - here, because Ctrl is handled
                          // diffferently on different terminals.`
  case '+':
    return Key::ZOOM_IN;
  }

  switch (buf[2]) {
  case 'A':
    return Key::UP;
  case 'B':
    return Key::DOWN;
  case 'C':
    return Key::RIGHT;
  case 'D':
    return Key::LEFT;
  }
  return Key::OTHER;
}

class KeyEventHandler {
  std::unordered_map<Key, std::unique_ptr<Command>> commands;

public:
  void addCommand(Key key, std::unique_ptr<Command> command) {
    commands[key] = std::move(command);
  }

public:
  void handleKeyPress() {
    Key key = readKey();
    if (key != Key::NONE && key != Key::OTHER) {
      commands.at(key)->execute();
    }
  }
};

#endif