#pragma once
#include "math_utils.h"
#include <string>
#include <vector>
template <typename T> struct BodyParams {
  Vec2<T> m_pos{};
  Vec2<T> m_vel{};
  Vec2<T> m_acc{};
  T m_mass{};
  T radius{};
};

template <typename T> struct SimulationParams {
  std::vector<BodyParams<T>> bodies;
  std::string name;
};

SimulationParams<double> parseYAML(const std::string &path);
