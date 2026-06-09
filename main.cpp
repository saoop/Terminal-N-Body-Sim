#include <iostream>
#include <unistd.h>
#include <vector>

#include "commands.h"
#include "input_controller.h"
#include "math_utils.h"
#include "rendering/renderer.h"
#include "simulation.h"
#include "utils.h"

int main() {
  RawMode raw_mode; // RAII for raw mode
  Simulation<double> sim{};
  Renderer renderer{125, 35};
  Menu menu{};
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

  renderer.start();
  while (true) {
    fpsController.startFrame();

    menu.render(fpsController.getCurrentFPS(), sim.getBodies().size());
    renderer.render(sim.getBodies());

    // Moving the cursor of the renderers
    menu.clear();

    renderer.clear();
    sim.step();

    fpsController.endFrame();

    keyEventHandler.handleKeyPress();
  }
  return 0;
}