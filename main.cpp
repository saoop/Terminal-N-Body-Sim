#include <iostream>
#include <unistd.h>
#include <vector>

#include "commands.h"
#include "input_controller.h"
#include "math_utils.h"
#include "rendering/windows.h"
#include "simulation.h"
#include "utils.h"
#include <memory>

#include <sys/ioctl.h>

int main() {
  constexpr double SUN_MASS = 1.989e30;
  constexpr double EARTH_MASS = 5.972e24;
  constexpr double EARTH_SUN_DIST = 1.496e11;     // 1 AU in meters
  constexpr double EARTH_ORBITAL_VEL = 29783.0;   // m/s
  constexpr double JUPITER_SUN_DIST = 7.785e11;   // ~5.2 AU
  constexpr double JUPITER_ORBITAL_VEL = 13070.0; // m/s
  constexpr double JUPITER_MASS = 1.898e27;
  constexpr double MOON_MASS = 7.342e22;
  constexpr double MOON_EARTH_DIST = 3.844e8;
  constexpr double MOON_ORBITAL_VEL = 1022.0; // m/s relative to Earth

  struct winsize w;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  RawMode raw_mode; // RAII for raw mode
  // Simulation<double> sim{2629743 / 1000};
  // BruteForceForcesComputer<double> forceComputer{100};
  Simulation<double> sim{
      std::make_unique<BruteForceForcesComputer<double>>(100), 1};
  ResourcesWindow resourcesWindow{0, 0, w.ws_col - 2, 1};

  SimulationWindow renderer{0, 3, w.ws_col * 0.7 - 3, w.ws_row - 6};
  MetricsWindow sideInfo{w.ws_col * 0.7 - 1, 3, w.ws_col * 0.3, w.ws_row - 6};

  // Timer timer {};

  FPSController fpsController{100, 10000};

  KeyEventHandler keyEventHandler{};

  // Moving camera
  keyEventHandler.addCommand(Key::UP,
                             std::make_unique<MoveCommand>(renderer, 0, -1));
  keyEventHandler.addCommand(Key::DOWN,
                             std::make_unique<MoveCommand>(renderer, 0, 1));
  keyEventHandler.addCommand(Key::RIGHT,
                             std::make_unique<MoveCommand>(renderer, 1, 0));
  keyEventHandler.addCommand(Key::LEFT,
                             std::make_unique<MoveCommand>(renderer, -1, 0));

  // Pause the simulation
  keyEventHandler.addCommand(Key::SPACE,
                             std::make_unique<PauseCommand<double>>(sim));

  // Zooming in and out
  keyEventHandler.addCommand(Key::ZOOM_IN,
                             std::make_unique<ZoomCommand>(renderer, 0.5));
  keyEventHandler.addCommand(Key::ZOOM_OUT,
                             std::make_unique<ZoomCommand>(renderer, 2));

  // Randomly initialize some bodies

  // Create some bodies to the left
  for (int i{0}; i < 300; i++) {
    sim.addBody({{-10000 + 200 * (rand() % 101), (rand() % 101) * 100},
                 {0, 0},
                 {0, 0},
                 {2000},
                 100});
  }

  sim.addBody({{0, 0}, {0, 0}, {0, 0}, {100000}, 300});

  // create some bodies to the right
  for (int i{0}; i < 300; i++) {
    sim.addBody({{10000 + 200 * (rand() % 101), (rand() % 101) * 100},
                 {0, 0},
                 {0, 0},
                 {2000},
                 100});
  }

  // sim.addBody({{0, 0}, {0, 0}, {0, 0}, SUN_MASS, EARTH_SUN_DIST / 10});
  // sim.addBody({{EARTH_SUN_DIST, 0},
  //              {0, EARTH_ORBITAL_VEL},
  //              {0, 0},
  //              EARTH_MASS,
  //              EARTH_SUN_DIST / 100}); // just for showcase
  // sim.addBody({{JUPITER_SUN_DIST, 0},
  //              {0, JUPITER_ORBITAL_VEL},
  //              {0, 0},
  //              JUPITER_MASS,
  //              JUPITER_SUN_DIST / 100});

  // sim.addBody({{EARTH_SUN_DIST, MOON_EARTH_DIST},
  //              {MOON_ORBITAL_VEL, EARTH_ORBITAL_VEL},
  //              {0, 0},
  //              MOON_MASS,
  //              EARTH_SUN_DIST / 200});

  // sim.addBody({{E}})

  // sim.addBody(
  //     {
  //         {0, 0}, {0, 0}, {0,0},   {22000}, 300
  //     }

  // );
  // sim.addBody(
  //      {
  //         {7000, 0}, {0, 0}, {0,0} , {22000}, 300
  //     }
  // );

  // std::cout << "SIZE: " << s.size() << "\n";
  std::cout << "\033[2J"; // clear entire screen once
  std::cout << "\033[H";  // move to top left once
  std::cout << "\033[s";  // save this as our origin
  while (true) {

    fpsController.startFrame();

    resourcesWindow.render(fpsController.getCurrentFPS());
    renderer.render(sim.getBodies());
    sideInfo.render(sim.getBodies().size(), 11);

    sim.step();

    fpsController.endFrame();

    keyEventHandler.handleKeyPress();
  }
  return 0;
}