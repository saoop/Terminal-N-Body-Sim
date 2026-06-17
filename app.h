#pragma once
#include "input_controller.h"
#include "states/state_base.h"
#include <memory>
class App {
  RawMode raw_mode; // RAII for raw mode

  std::unique_ptr<State> m_current_state;

public:
  void setState(std::unique_ptr<State> state) {
    m_current_state = std::move(state);
  }
  void run() {
    while (true) {
      m_current_state->update();
    }
  }
};