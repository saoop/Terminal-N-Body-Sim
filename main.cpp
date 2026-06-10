#include <iostream>
#include <unistd.h>
#include <vector>

#include "commands.h"
#include "input_controller.h"
#include "math_utils.h"
#include "rendering/renderer.h"
#include "simulation.h"
#include "utils.h"

#include <sys/ioctl.h>

int main() {
  struct winsize w;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  RawMode raw_mode; // RAII for raw mode
  Simulation<double> sim{};
  ResourcesWindow resourcesWindow{0, 0, w.ws_col - 2, 1};

  Renderer renderer{0, 3, w.ws_col * 0.7 - 3, w.ws_row - 6};
  SideInfo sideInfo{w.ws_col * 0.7 - 1, 3, w.ws_col * 0.3, w.ws_row - 6};

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
  for (int i{0}; i < 62; i++) {
    sim.addBody({{-10000 + 200 * (rand() % 101), (rand() % 101) * 100},
                 {0, 0},
                 {0, 0},
                 {2000},
                 100});
  }

  sim.addBody({{0, 0}, {0, 0}, {0, 0}, {100000}, 300});

  // create some bodies to the right
  for (int i{0}; i < 62; i++) {
    sim.addBody({{10000 + 200 * (rand() % 101), (rand() % 101) * 100},
                 {0, 0},
                 {0, 0},
                 {2000},
                 100});
  }

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