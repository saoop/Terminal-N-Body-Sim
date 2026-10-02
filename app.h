#pragma once
#include "states/state_base.h"
#include <memory>

class App {

  std::unique_ptr<State> m_current_state;
  std::unique_ptr<State> m_pending_state;

public:
  void setState(std::unique_ptr<State> state) {
    m_pending_state = std::move(state);
  }
  void run() {
    while (true) {
      if (m_pending_state) {
        m_current_state = std::move(m_pending_state);
        // m_pending_state
      }
      m_current_state->update();
    }
  }
};