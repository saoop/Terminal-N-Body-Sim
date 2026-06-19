#ifndef BUTTON_H
#define BUTTON_H
#include <functional>
#include <iostream>
#include <memory>
#include <string>

class Button {
private:
  int m_x;
  int m_y;
  int m_width;
  int m_height;
  std::string m_text;
  std::function<void()> m_on_press;
  std::string m_tag;
  // std::unique_ptr<Command> m_command;

  // bool m_selected = false;
  void printTruncated(const std::string &s, int max_width) const {
    if (static_cast<int>(s.size()) < max_width) {
      std::cout << s;
      return;
    }
    std::cout << s.substr(0, max_width - 1) + "…";
  }

public:
  Button(int x, int y, int width, int height, std::string text,
         std::function<void()> on_press, std::string tag = "default")
      : m_x{x}, m_y{y}, m_width{width}, m_height{height}, m_text{text},
        m_on_press{on_press}, m_tag{tag} {}
  // void select() { m_selected = !m_selected; }
  void render(bool selected) {
    if (selected) {
      printTruncated("> " + m_text, m_width);
    } else {
      printTruncated("  " + m_text, m_width);
    }
  }
  void press() { m_on_press(); };
  std::string const &getTag() { return m_tag; }
};
#endif