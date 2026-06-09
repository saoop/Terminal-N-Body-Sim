
#ifndef DISPLAY_H
#define DISPLAY_H

#include "../math_utils.h"
#include "../simulation.h" //TODO Put stuff in common + we only need pos and rad not the whole reference?
#include "consts.h"
#include "grid.h"
#include <iostream>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <vector>

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
  RawMode() { enableRawMode(original); }
  ~RawMode() { disableRawMode(original); }
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

class Renderer {
  /*
  This class defines the environment variables such as pixel size, zoom etc.
  */
private:
  int m_width{};
  int m_height{};
  int m_center_x{};
  int m_center_y{};
  double m_pixel_size{1000}; // in meters???
  int m_offset_x{0};
  int m_offset_y{0};

public:
  Renderer(int width, int height) : m_width{width}, m_height{height} {
    std::cout << "Initializing the Display..." << '\n';
    std::cout << std::endl;

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

  void start() {}

  void clear() const {
    for (int i{0}; i < m_height; i++) {
      std::cout << "\033[A\r";
    }

    // For the border??
    std::cout << "\033[A\r";
    std::cout << "\033[A\r";
  }

  void drawPixel(std::string &s) const { std::cout << s; }

  void drawTopBorder() const {
    std::cout << "┏";
    for (int i{1}; i < m_width + 1; i++) { // + 1 because of border
      std::cout << "━";
    }
    std::cout << "┓";
    std::cout << "\n";
  }

  void drawBottomBorder() const {
    std::cout << "┗";
    for (int i{1}; i < m_width + 1; i++) {
      std::cout << "━";
    }
    std::cout << "┛";
    std::cout << "\n";
  }

  void render(std::vector<CircleBody<double>> &bodies) const {
    drawTopBorder();

    // Grid of pixels.
    Grid grid{m_height, m_width};

    // std::vector<bool> grid(m_width * m_height, false); //
    // Check in what pixle each body lies.
    for (auto &body : bodies) {
      auto const &pos{body.getPos()};

      int ind_y{pos.y / m_pixel_size + m_center_y -
                m_offset_y}; // Convert them from 0 centered to grid coords.
      int ind_x{pos.x / m_pixel_size + m_center_x - m_offset_x};

      double radius = body.getRadius();

      double diameter = radius * 2;

      int radius_in_pixels = static_cast<int>(radius / m_pixel_size);

      // Check if some part of the body is visible
      if (ind_x + radius_in_pixels < 0 || ind_x - radius_in_pixels >= m_width ||
          ind_y + radius_in_pixels < 0 ||
          ind_y - radius_in_pixels >= m_height) {
        continue;
      }

      grid.addBody(ind_x, ind_y);

      if (diameter <= m_pixel_size) {
        int times = static_cast<int>(diameter / m_pixel_size);

        times = std::min(times, Pixels::MAX_INTENSITY);
        times = std::max(times, 1);

        grid.addIntensity(ind_x, ind_y, times);
      } else {
        grid.addIntensity(ind_x, ind_y, Pixels::MAX_INTENSITY);
        // Draw a circle

        // IDK how to do it efficiently...
        double r_squared = radius_in_pixels * radius_in_pixels;

        for (int i = 0; i < radius_in_pixels * 2 + 1; i++) {
          for (int j = 0; j < radius_in_pixels * 2 + 1; j++) {
            int x{i - radius_in_pixels};
            int y{j - radius_in_pixels};

            // Can use map 0.01, 0.04, 0.09 ... -> squares of usual 0.1, 0.2,
            // 0.3, etc... To render the intensity of the pixel (center ->
            // stronger)

            if (x * x + y * y <= r_squared) {
              // std::cout << "Setting circle intensity...";
              grid.addIntensity(ind_x + x, ind_y + y, 6);
            }
          }
        }
      }
    }

    // Drawing the bodies
    for (int y{0}; y < m_height; y++) {
      std::cout << "┃";

      for (int x{0}; x < m_width; x++) {
        if (grid.getNumBodies(x, y) == 0) {
          std::cout << Pixels::one_body_in_pixel.at(grid.getIntensity(x, y));

        } else if (grid.getNumBodies(x, y) == 1) {
          // std::cout << grid.getIntensity(x, y);
          std::cout << Pixels::one_body_in_pixel.at(grid.getIntensity(x, y));
        } else {
          std::cout << Pixels::several_bodies_in_pixel.at(
              grid.getNumBodies(x, y));
        }
      }
      std::cout << "┃";
      std::cout << "\n";
    }
    drawBottomBorder();
    // clear();
  }
};

class Menu {
  // private:

public:
  void clear() { std::cout << "\033[A\r"; }
  void render(int fps, int number_of_bodies) {
    std::cout << "FPS: " << fps << " | Number of bodies: " << number_of_bodies
              << "\n";
  }
};

#endif // DISPLAY_H
