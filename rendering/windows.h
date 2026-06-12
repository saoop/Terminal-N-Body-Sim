#ifndef DISPLAY_H
#define DISPLAY_H

#include "../math_utils.h"
#include "../simulation.h"
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

class ResourcesWindow : public Window {
public:
  ResourcesWindow(int pos_x, int pos_y, int width, int height)
      : Window{pos_x, pos_y, width, height} {}

  void render(int fps) const {
    startRendering();
    drawFullBorder();
    moveCursor(1, 1);
    std::string s = "FPS : " + std::to_string(fps) + " | ";
    printTruncated(s, m_width);
    stopRendering();
  }
};

class MetricsWindow : public Window {
public:
  MetricsWindow(int pos_x, int pos_y, int width, int height)
      : Window{pos_x, pos_y, width, height} {}

  void render(size_t num_bodies, double total_energy) const {
    // Line 0 — body count
    startRendering();
    drawFullBorder();
    moveCursor(1, 1); // boder
    std::string bodies_string = "Bodies: " + std::to_string(num_bodies);
    printTruncated(bodies_string, m_width);

    // Line 1 — energy
    moveCursor(1, 2);

    std::string energy_string = "Energy: " + std::to_string(total_energy);
    printTruncated(energy_string, m_width);
    stopRendering();
  }
};

class SimulationWindow : public Window {
private:
  int m_center_x{};
  int m_center_y{};
  double m_pixel_size{1.496e11 / 5}; // 1 AU in meters};
  int m_offset_x{0};
  int m_offset_y{0};

public:
  SimulationWindow(int pos_x, int pos_y, int width, int height)
      : Window{pos_x, pos_y, width, height} {
    m_center_x = m_width / 2;
    m_center_y = m_height / 2;
  }

  void nudge(int offset_x, int offset_y) {
    m_offset_x += offset_x;
    m_offset_y += offset_y;
  }

  double getPixelSize() const { return m_pixel_size; }
  void setPixelSize(double new_size) { m_pixel_size = new_size; }
  void zoom(double factor) { setPixelSize(m_pixel_size * factor); }

  void render(std::vector<CircleBody<double>> &bodies) const {
    startRendering();

    drawFullBorder();
    moveCursor(m_pos_x, m_pos_y);
    // Build grid
    Grid grid{m_height, m_width};

    for (auto &body : bodies) {
      auto const &pos{body.getPos()};

      int ind_y =
          static_cast<int>(pos.y / m_pixel_size) + m_center_y - m_offset_y;
      int ind_x =
          static_cast<int>(pos.x / m_pixel_size) + m_center_x - m_offset_x;

      double radius = body.getRadius();
      double diameter = radius * 2;
      int radius_in_pixels = static_cast<int>(radius / m_pixel_size);

      // Visibility check
      if (!grid.isInBounds(ind_x, ind_y, radius_in_pixels))
        continue;

      grid.addBody(ind_x, ind_y);

      if (diameter <= m_pixel_size) {
        int times = static_cast<int>(diameter / m_pixel_size);
        grid.addIntensity(ind_x, ind_y, times);
      } else {
        grid.drawCircle(ind_x, ind_y, radius_in_pixels);
      }
    }

    // Draw the grid
    for (int y = 0; y < m_height; y++) {
      moveCursor(1, 1 + y); // +1 for top border
      std::string buff;
      buff.reserve(m_width);

      for (int x = 0; x < m_width; x++) {
        int num_bodies = grid.getNumBodies(x, y);
        if (num_bodies <= 1) {
          buff += one_body_in_pixel.at(grid.getIntensity(x, y));
        } else {
          buff += several_bodies_in_pixel.at(num_bodies);
        }
      }
      std::cout << buff;
      stopRendering();
    }
  }
};

// class WindowManager {
// private:
//   std::vector<Window &> m_windows;

// public:
//   void addWindow(Window &new_window) { m_windows.push_back(new_window); }
//   void render() {
//     for (auto &window : m_windows) {
//       window.render();
//     }
//   }
// };

#endif // DISPLAY_H