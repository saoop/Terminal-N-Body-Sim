#ifndef COMMANDS_H
#define COMMANDS_H

#include "renderer.h"
#include "simulation.h"

struct Command{
    virtual void execute() const = 0;
};


struct MoveCommand : Command{
    // CommandName name {CommandName::MOVE};
    Renderer & renderer;
    int x {};
    int y {};
    MoveCommand(Renderer & renderer, int x, int y)
    : renderer{renderer}
    , x{x}
    , y{y}
    {

    }

    void execute() const override{
        renderer.nudge(x, y);
    }
};

template <typename T>
struct PauseCommand : Command {
    // CommandName name {CommandName::PAUSE};

    Simulation<T> & sim;
    PauseCommand(Simulation<T> & sim)
    : sim{sim}
    {  }

    void execute() const override{
        sim.togglePause();
    }
};


struct ZoomCommand : Command {
    Renderer & renderer;
    double factor;
    ZoomCommand(Renderer & renderer, double factor)
    : renderer{renderer}
    , factor{factor}
    {
        
    }

    void execute() const override  {
        renderer.zoom(factor);
    }

};

#endif