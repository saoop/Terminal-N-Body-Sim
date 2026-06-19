#ifndef CONSTS_H
#define CONSTS_H

#include <map>

namespace pixels {
inline const std::map<int, const char *> one_body_in_pixel{
    {0, " "}, {1, "."}, {2, "▪"}, {3, "▣"}, {4, "■"}, {5, "∎"}, {6, "█"}};

inline const std::map<int, const char *> several_bodies_in_pixel{
    {0, " "}, {1, "."}, {2, ":"}, {3, "⠪"},  {4, "⠭"},  {5, "⠷"},  {6, "⠿"},
    {7, "⡿"}, {8, "⣿"}, {9, "░"}, {10, "▒"}, {11, "▓"}, {12, "█"},
};

inline const char *font_pixel = "█";

inline const int MAX_BODIES{12};
inline const int MAX_INTENSITY{6};

enum class Border {
  VERTICAL,
  HORIZONTAL,
  UPPER_RIGHT,
  UPPER_LEFT,
  BOTTOM_RIGHT,
  BOTTOM_LEFT
};

inline const std::map<Border, const char *> border_pixels{
    {Border::VERTICAL, "┃"},
    {Border::HORIZONTAL, "━"},
    {Border::UPPER_LEFT, "┏"},
    {Border::UPPER_RIGHT, "┓"},
    {Border::BOTTOM_LEFT, "┗"},
    {Border::BOTTOM_RIGHT, "┛"}

};
} // namespace pixels
// namespace Pixels

#endif
