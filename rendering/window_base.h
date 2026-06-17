#pragma once

#include "../math_utils.h"
#include "../simulation/bodies.h"
#include "button.h"
#include "consts.h"
#include "grid.h"
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

using namespace pixels;

class Window {
protected:
  int m_pos_x{}; // upper left corner
  int m_pos_y{};
  int m_width{};
  int m_height{};
  void moveCursor(int x, int y) const {
    // ANSI: move to row y+1, col x+1 (1-indexed)
    std::cout << "\033[" << (m_pos_y + y + 1) << ";" << (m_pos_x + x + 1)
              << "H";
  }

  void startRendering() const {
    std::cout << "\033[s"; // save cursor position
  }

  void stopRendering() const {
    std::cout << "\033[u"; // restore cursor position
    std::cout.flush();
  }

  // Truncate string to fit window width.
  // CAUTION: only works correctly with plain ASCII text, not multi-byte UTF-8.
  void printTruncated(const std::string &s, int max_width) const {
    if (static_cast<int>(s.size()) < max_width) {
      std::cout << s;
      return;
    }
    std::cout << s.substr(0, max_width - 1) + "…";
  }
  void drawTopBorder() const {
    moveCursor(0, 0);
    std::cout << border_pixels.at(Border::UPPER_LEFT);
    for (int i = 0; i < m_width; i++)
      std::cout << border_pixels.at(Border::HORIZONTAL);
    std::cout << border_pixels.at(Border::UPPER_RIGHT);
  }

  void drawBottomBorder() const {
    moveCursor(0, m_height + 1);
    std::cout << border_pixels.at(Border::BOTTOM_LEFT);
    for (int i = 0; i < m_width; i++)
      std::cout << border_pixels.at(Border::HORIZONTAL);
    std::cout << border_pixels.at(Border::BOTTOM_RIGHT);
  }

  void drawSideBorder() const {
    for (int i = 1; i < m_height + 1; i++) {
      moveCursor(0, i);
      std::cout << border_pixels.at(Border::VERTICAL);
      moveCursor(1 + m_width, i);
      std::cout << border_pixels.at(Border::VERTICAL);
    }
  }

  void drawFullBorder() const {
    drawTopBorder();
    drawBottomBorder();
    drawSideBorder();
  }

public:
  Window(int pos_x, int pos_y, int width, int height)
      : m_pos_x{pos_x}, m_pos_y{pos_y}, m_width{width}, m_height{height} {}
};