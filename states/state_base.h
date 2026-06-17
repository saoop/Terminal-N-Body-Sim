#pragma once
class State {
public:
  State() = default;
  virtual void update() = 0;
};
