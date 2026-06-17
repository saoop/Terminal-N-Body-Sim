#ifndef UTILS_H
#define UTILS_H
#include <bits/chrono.h>
#include <deque>
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
  std::deque<double> m_last_fps; // last 40
  static int const FPS_WINDOW_SIZE = 40;
  Timer m_timer{};

public:
  FPSController(int max_fps, int minimal_wait)
      : m_max_fps{max_fps}, m_minimal_wait{minimal_wait},
        m_max_frame_length{1000000 / max_fps} {}

  int getCurrentFPS() const { return m_current_fps; }

  void startFrame() { m_timer.start(); }

  void updateCurrentFPS(double new_fps) {
    // Uses moving average to reduce the "flickering" of the FPS indicator.
    m_last_fps.push_back(new_fps);
    if (m_last_fps.size() > FPS_WINDOW_SIZE) {
      m_last_fps.pop_front();
    }

    double avg{0};
    for (auto fps : m_last_fps)
      avg += fps;

    avg /= FPS_WINDOW_SIZE;

    m_current_fps = static_cast<int>(avg);
  }

  void endFrame() {
    m_timer.stop();

    auto to_sleep{std::max(0, m_minimal_wait - m_timer.elaspsed())};

    // Update the fps estimate
    double new_fps =
        1000000 / (to_sleep + m_timer.elaspsed()); // to Seconds conversion

    updateCurrentFPS(new_fps);

    // Sleep the rest of the frame
    usleep(to_sleep);
  }
};
#endif