#ifndef GRID_H
#define GRID_H

#include "consts.h"
#include <unistd.h>
#include <vector>

struct Grid {
  /*
  Character Grid. This is a class between simulation and rendering, that makes
  rendering easier.

  Current version uses a pair of 2 ints in the grid: number of bodies that
  should be displayed in one character and intensity of the pixel. If there are
  multiple bodies that fit into one character we can display them with something
  like ":" or "⠭". If there is only one body in the character, we use intensity
  to make it appear smaller/larger, depending on the relative size of body to
  the current pixel size.

  */
  int m_height{};
  int m_width{};
  int m_size{};

  // first body, second intensity.
  std::vector<std::pair<int, int>> m_grid;

  Grid(int height, int width)
      : m_height{height}, m_width{width}, m_size{m_height * m_width},
        m_grid(width * height, std::pair<int, int>(0, 0)) {}

  bool isValid(int x, int y) const {
    return y < m_height && x < m_width && y >= 0 && x >= 0;
  }

  void addBody(int x, int y) {
    if (!isValid(x, y)) {
      return; // Silent, since it is just for rendering.
    }
    m_grid[y * m_width + x].first =
        std::min(Pixels::MAX_BODIES, m_grid[y * m_width + x].first + 1);
  }

  void addIntensity(int x, int y, int val) {
    if (!isValid(x, y)) {
      return; // Silent, since it is just for rendering.
    }

    m_grid[y * m_width + x].second =
        std::min(Pixels::MAX_INTENSITY, m_grid[y * m_width + x].second + val);
  }

  int getNumBodies(int x, int y) const {
    if (!isValid(x, y)) {
      return 0; // Silent, since it is just for rendering.
    }
    return m_grid[y * m_width + x].first;
  }

  int getIntensity(int x, int y) const {
    if (!isValid(x, y)) {
      return 0; // Silent, since it is just for rendering.
    }
    return m_grid[y * m_width + x].second;
  }
};

#endif