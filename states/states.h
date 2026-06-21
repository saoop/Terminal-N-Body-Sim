#ifndef STATES_H
#define STATES_H
#include <filesystem>
#include <format>
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
  SimulationState(std::string filename) {
    // read the configuration file

    SimulationParams<double> simParams = parseYAML(filename);

    constexpr double G = 100;

    struct winsize w;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    // Build simulation from parameters
    if (simParams.forceCalcType == ForceCalcType::BarnesHut) {
      auto force_computer =
          std::make_unique<BarnesHutsForcesComputer<double>>(simParams.G);
      sim = std::make_unique<Simulation<double>>(std::move(force_computer),
                                                 simParams.dt);
    } else if (simParams.forceCalcType == ForceCalcType::BruteForce) {
      auto force_computer =
          std::make_unique<BruteForceForcesComputer<double>>(simParams.G);
      sim = std::make_unique<Simulation<double>>(std::move(force_computer),
                                                 simParams.dt);
    }

    // create bodies
    for (auto bodyParam : simParams.bodies) {
      sim->addBody({bodyParam.m_pos, bodyParam.m_vel, bodyParam.m_acc,
                    bodyParam.m_mass, bodyParam.radius});
    }

    // Rendering
    resourcesWindow = std::make_unique<ResourcesWindow>(0, 0, w.ws_col - 2, 1);

    renderer = std::make_unique<SimulationWindow>(0, 3, w.ws_col * 0.7 - 3,
                                                  w.ws_row - 6);

    sideInfo = std::make_unique<MetricsWindow>(w.ws_col * 0.7 - 1, 3,
                                               w.ws_col * 0.3, w.ws_row - 6);

    fpsController = std::make_unique<FPSController>(100, 10000);

    // Key press handling.
    keyEventHandler = std::make_unique<KeyEventHandler>();

    keyEventHandler->addCallback(Key::UP, [&]() { renderer->nudge(0, -1); });

    keyEventHandler->addCallback(Key::DOWN, [&]() { renderer->nudge(0, 1); });
    keyEventHandler->addCallback(Key::RIGHT, [&]() { renderer->nudge(1, 0); });
    keyEventHandler->addCallback(Key::LEFT, [&]() { renderer->nudge(-1, 0); });

    // Pause the simulation
    keyEventHandler->addCallback(Key::SPACE, [&]() { sim->togglePause(); });

    // Zooming in and out
    keyEventHandler->addCallback(Key::ZOOM_IN, [&]() { renderer->zoom(0.5); });
    keyEventHandler->addCallback(Key::ZOOM_OUT, [&]() { renderer->zoom(2); });
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
  struct winsize w;
  App &m_app;
  std::string m_chosen_file;

  // Displays the yaml files as buttons
  void closeDisplayFiles() {
    startingWindow->removeButtonByTag("file_button");
    startingWindow->resetSelected();
    startingWindow->reset();
  }
  void setChosenFile(std::string chosen_file) {
    // m_chosen_file = chosen_file.copy();
  }

  void displayFiles() {
    int i{2};
    std::cout << "object at: " << this << "\n"; // when creating button

    for (auto &entry : std::filesystem::directory_iterator("../scenarios/")) {
      auto path = std::filesystem::absolute(entry.path()).string();
      auto filename = entry.path().filename().string();
      startingWindow->addButton(std::make_unique<Button>(
          1, i++, w.ws_col - 3, 1, std::format("  {}", filename),
          [this, path, filename]() {
            std::cout << "this in lambda: " << this
                      << "\n"; // when button pressed
            // parseYAML(std::filesystem::absolute(path).string());
            m_chosen_file = path;

            closeDisplayFiles();
          },
          "file_button"));
    }
  }

public:
  MenuState(App &app) : m_app{app} {
    // struct winsize w;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    startingWindow =
        std::make_unique<StartingWindow>(0, 0, w.ws_col - 2, w.ws_row - 2);

    startingWindow->addButton(std::make_unique<Button>(
        1, 2, w.ws_col - 3, 1, "Start Simulation", [this]() {
          m_app.setState(std::make_unique<SimulationState>(m_chosen_file));
        }));

    startingWindow->addButton(std::make_unique<Button>(
        1, 1, w.ws_col - 3, 1, "Choose a file", [&]() { displayFiles(); }));

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