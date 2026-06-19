#ifndef DISPLAY_H
#define DISPLAY_H

#include "../math_utils.h"
#include "../simulation/bodies.h"
#include "button.h"
#include "consts.h"
#include "grid.h"
#include "utils.h"
#include "window_base.h"
#include <format>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

using namespace pixels;

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
  double m_pixel_size{1.496e11 / 5}; // 1 AU in meters};
  int m_offset_x{0};
  int m_offset_y{0};

public:
  SimulationWindow(int pos_x, int pos_y, int width, int height,
                   double pixel_size = 1000)
      : Window{pos_x, pos_y, width, height} {
    clearFullTerminal();
    m_pixel_size = pixel_size;
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

class StartingWindow : public Window {
private:
  std::vector<std::unique_ptr<Button>> m_buttons;
  int m_selected{};

  void drawWelcomeText() {
    int width_of_text = 10;
    const char *text = "░▒▓█ TERMINAL N-BODY SIMULATION █▓▒░\n";
    int starting_point{m_center_x - 36 / 2};
    moveCursor(starting_point, 1);
    std::cout << text;
  }

public:
  StartingWindow(int pos_x, int pos_y, int width, int height)
      : Window{pos_x, pos_y, width, height} {
    clearFullTerminal();
  }

  void resetSelected() { m_selected = 0; }
  void reset() { clearFullTerminal(); }

  void addButton(std::unique_ptr<Button> button) {
    m_buttons.push_back(std::move(button));
  }

  void removeButton(int id) {
    auto it = m_buttons.begin() + id;
    m_buttons.erase(it);
  }

  void removeButtonByTag(std::string tag) {
    std::erase_if(m_buttons,
                  [tag](const auto &b) { return tag == b->getTag(); });
  }

  void selectNext() {
    // std::cout << "next";
    m_selected = m_selected >= m_buttons.size() - 1 ? 0 : m_selected + 1;
  }

  void selectPrevious() {
    // std::cout << "lol";
    m_selected = m_selected <= 0 ? m_buttons.size() - 1 : m_selected - 1;
  }

  void pressButton() { m_buttons.at(m_selected)->press(); }

  void render() {
    startRendering();

    drawFullBorder();
    drawWelcomeText();

    moveCursor(m_pos_x, m_pos_y);
    for (int i = 0; i < m_buttons.size(); i++) {
      moveCursor(1, i + 2);
      // if ()
      m_buttons.at(i)->render(m_selected == i);
    }
    stopRendering();
  }
};

#endif // DISPLAY_H