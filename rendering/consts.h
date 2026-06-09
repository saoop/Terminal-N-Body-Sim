#ifndef CONSTS_H
#define CONSTS_H

#include <map>

enum class Key { UP, DOWN, RIGHT, LEFT, SPACE, ZOOM_IN, ZOOM_OUT, OTHER, NONE };

namespace Pixels {
inline const std::map<int, const char *> one_body_in_pixel{
    {0, " "}, {1, "."}, {2, "▪"}, {3, "▣"}, {4, "■"}, {5, "∎"}, {6, "█"}};

inline const std::map<int, const char *> several_bodies_in_pixel{
    {0, " "}, {1, "."}, {2, ":"}, {3, "⠪"},  {4, "⠭"},  {5, "⠷"},  {6, "⠿"},
    {7, "⡿"}, {8, "⣿"}, {9, "░"}, {10, "▒"}, {11, "▓"}, {12, "█"},
};

inline static const int MAX_BODIES{9};
inline static const int MAX_INTENSITY{6};
} // namespace Pixels

#endif