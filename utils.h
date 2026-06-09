#include <bits/chrono.h>
#include <memory>
#include <unistd.h>

class Timer {
private:
  std::chrono::_V2::steady_clock::time_point m_start;
  std::chrono::_V2::steady_clock::time_point m_end;

public:
  std::chrono::_V2::steady_clock::time_point start() {
    m_start = std::chrono::steady_clock::now();
    return m_start;
  }

  std::chrono::_V2::steady_clock::time_point stop() {
    m_end = std::chrono::steady_clock::now();
    return m_end;
  }

  int elaspsed() const {
    if (m_start > m_end) {
      return -1;
    }
    return std::chrono::duration_cast<std::chrono::microseconds>(m_end -
                                                                 m_start)
        .count();
  }
};

class FPSController {
private:
  int m_max_fps{};          // in 1/Seconds
  int m_minimal_wait{};     // In Microseconds
  int m_max_frame_length{}; // In Microseconds
  int m_current_fps{};
  Timer m_timer{};

public:
  FPSController(int max_fps, int minimal_wait)
      : m_max_fps{max_fps}, m_minimal_wait{minimal_wait},
        m_max_frame_length{1000000 / max_fps} {}

  int getCurrentFPS() const { return m_current_fps; }

  void startFrame() { m_timer.start(); }

  void endFrame() {
    m_timer.stop();

    auto to_sleep{std::max(0, m_minimal_wait - m_timer.elaspsed())};

    // Update the fps
    m_current_fps =
        1000000 / (to_sleep + m_timer.elaspsed()); // to Seconds conversion

    // Sleep the rest of the frame
    usleep(to_sleep);
  }
};