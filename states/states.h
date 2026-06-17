#ifndef STATES_H
#define STATES_H
#include <iostream>
#include <unistd.h>
#include <vector>

#include "../app.h"
#include "../input_controller.h"
#include "../math_utils.h"
#include "../rendering/windows.h"
#include "../simulation/bodies.h"
#include "../simulation/force_computers.h"
#include "../simulation/simulation.h"
#include "../utils.h"
#include "state_base.h"
#include <cmath>
#include <memory>
#include <sys/ioctl.h>

class SimulationState : public State {
private:
  std::unique_ptr<Simulation<double>> sim;
  //   RawMode raw_mode; // RAII for raw mode
  std::unique_ptr<ResourcesWindow> resourcesWindow;
  std::unique_ptr<SimulationWindow> renderer;
  std::unique_ptr<MetricsWindow> sideInfo;
  std::unique_ptr<KeyEventHandler> keyEventHandler;
  std::unique_ptr<FPSController> fpsController;

public:
  SimulationState(int fd = 0) {
    // read the configuration file

    // create bodies
    constexpr double G = 100;

    struct winsize w;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    // Simulation<double> sim{2629743 / 1000};
    // BruteForceForcesComputer<double> forceComputer{100};
    auto force_computer = std::make_unique<BarnesHutsForcesComputer<double>>(G);
    sim = std::make_unique<Simulation<double>>(std::move(force_computer), 1);

    resourcesWindow = std::make_unique<ResourcesWindow>(0, 0, w.ws_col - 2, 1);

    renderer = std::make_unique<SimulationWindow>(0, 3, w.ws_col * 0.7 - 3,
                                                  w.ws_row - 6);

    // startingWindow = {0, 0, w.ws_col - 2, w.ws_row - 2};

    sideInfo = std::make_unique<MetricsWindow>(w.ws_col * 0.7 - 1, 3,
                                               w.ws_col * 0.3, w.ws_row - 6);

    // Timer timer {};

    fpsController = std::make_unique<FPSController>(100, 10000);

    keyEventHandler = std::make_unique<KeyEventHandler>();

    // TODO: commands -> lambdas.
    // Moving camera

    keyEventHandler->addCallback(Key::UP, [&]() { renderer->nudge(0, -1); });

    keyEventHandler->addCallback(Key::DOWN, [&]() { renderer->nudge(0, 1); });
    keyEventHandler->addCallback(Key::RIGHT, [&]() { renderer->nudge(1, 0); });
    keyEventHandler->addCallback(Key::LEFT, [&]() { renderer->nudge(-1, 0); });

    // Pause the simulation
    keyEventHandler->addCallback(Key::SPACE, [&]() { sim->togglePause(); });

    // Zooming in and out
    keyEventHandler->addCallback(Key::ZOOM_IN, [&]() { renderer->zoom(0.5); });
    keyEventHandler->addCallback(Key::ZOOM_OUT, [&]() { renderer->zoom(2); });

    // Black hole / or sun
    double M = 10000;
    double R = 5000;
    double M_planet = M * 0.0001;
    double vel_offset = 10;
    double x_offset = 40000;
    double y_offset = 2000;

    sim->addBody({{-x_offset, -y_offset}, {vel_offset, 0}, {0, 0}, {M}, 400});
    sim->addBody({{x_offset, y_offset}, {-vel_offset, 0}, {0, 0}, {M}, 400});

    // Add bodies around the sun
    for (int i = 0; i < 5000; i++) {
      double rad = 2 * 3.14 * ((double)rand() / RAND_MAX);
      double r = R + 200 * (rand() % 100);
      double x = std::cos(rad);
      double y = std::sin(rad);
      double v = std::sqrt(G * M / r);
      double vel_x = -y;
      double vel_y = x;

      sim->addBody({{x_offset + r * x, y_offset + r * y},
                    {-vel_offset + v * vel_x, v * vel_y},
                    {0, 0},
                    {M_planet},
                    {100}});
      sim->addBody({{-x_offset + r * x, -y_offset + r * y},
                    {vel_offset + v * vel_x, v * vel_y},
                    {0, 0},
                    {M_planet},
                    {100}});
    }
  }

  void update() override {
    fpsController->startFrame();

    resourcesWindow->render(fpsController->getCurrentFPS());
    renderer->render(sim->getBodies());
    sideInfo->render(sim->getBodies().size(), 11);

    sim->step();

    fpsController->endFrame();

    keyEventHandler->handleKeyPress();
  }
};

class MenuState : public State {
private:
  int current_index;

  std::unique_ptr<StartingWindow> startingWindow;
  std::unique_ptr<KeyEventHandler> keyEventHandler;

  App &m_app;

public:
  MenuState(App &app) : m_app{app} {
    struct winsize w;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    startingWindow =
        std::make_unique<StartingWindow>(0, 0, w.ws_col - 2, w.ws_row - 2);

    startingWindow->addButton(
        std::make_unique<Button>(1, 1, w.ws_col - 3, 1, "Choose a file", [&]() {
          m_app.setState(std::make_unique<SimulationState>());
        }));

    startingWindow->addButton(std::make_unique<Button>(
        1, 2, w.ws_col - 3, 1, "Start Simulation",
        [&]() { m_app.setState(std::make_unique<SimulationState>()); }));

    keyEventHandler = std::make_unique<KeyEventHandler>();
    keyEventHandler->addCallback(Key::DOWN,
                                 [&]() { startingWindow->selectNext(); });
    keyEventHandler->addCallback(Key::UP,
                                 [&]() { startingWindow->selectPrevious(); });
    keyEventHandler->addCallback(Key::ENTER,
                                 [&]() { startingWindow->pressButton(); });
  }
  void update() override {
    startingWindow->render();

    keyEventHandler->handleKeyPress();
  }
};
#endif