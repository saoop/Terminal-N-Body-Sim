#ifndef COMMANDS_H
#define COMMANDS_H

#include "rendering/windows.h"
#include "simulation/simulation.h"

struct Command {
  virtual void execute() const = 0;
};

struct MoveCommand : Command {
  // CommandName name {CommandName::MOVE};
  SimulationWindow &renderer;
  int x{};
  int y{};
  MoveCommand(SimulationWindow &renderer, int x, int y)
      : renderer{renderer}, x{x}, y{y} {}

  void execute() const override { renderer.nudge(x, y); }
};

template <typename T> struct PauseCommand : Command {
  // CommandName name {CommandName::PAUSE};

  Simulation<T> &sim;
  PauseCommand(Simulation<T> &sim) : sim{sim} {}

  void execute() const override { sim.togglePause(); }
};

struct ZoomCommand : Command {
  SimulationWindow &renderer;
  double factor;
  ZoomCommand(SimulationWindow &renderer, double factor)
      : renderer{renderer}, factor{factor} {}

  void execute() const override { renderer.zoom(factor); }
};

#endif