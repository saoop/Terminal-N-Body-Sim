#include <iostream>
#include <vector>
#include "math_utils.h"
#include "simulation.h"
#include "renderer.h"
#include <unordered_map>
#include <memory>
#include "commands.h"


class KeyEventHandler { 
    std::unordered_map<Key, std::unique_ptr<Command>> commands;

    public:
    void addCommand(Key key, std::unique_ptr<Command> command){
        commands[key] = std::move(command);
    }

    public:
    void handleKeyPress(){
        Key key = readKey();
        if (key != Key::NONE && key != Key::OTHER){
            commands.at(key)->execute();
        }
    }
};

int main() {
    RawMode raw_mode; // RAII for raw mode
    Simulation<double> sim {};
    Renderer renderer {125, 35};

    KeyEventHandler keyEventHandler{};

    // Moving camera
    keyEventHandler.addCommand(Key::UP, std::make_unique<MoveCommand> (renderer, 0, -1));
    keyEventHandler.addCommand(Key::DOWN, std::make_unique<MoveCommand> (renderer, 0, 1));
    keyEventHandler.addCommand(Key::RIGHT, std::make_unique<MoveCommand> (renderer, 1, 0));
    keyEventHandler.addCommand(Key::LEFT, std::make_unique<MoveCommand> (renderer, -1, 0));

    // Pause the simulation
    keyEventHandler.addCommand(Key::SPACE, std::make_unique<PauseCommand<double>>(sim));

    //Zooming in and out
    keyEventHandler.addCommand(Key::ZOOM_IN, std::make_unique<ZoomCommand> (renderer, 0.5));
    keyEventHandler.addCommand(Key::ZOOM_OUT, std::make_unique<ZoomCommand> (renderer, 2));


    // Randomly initialize some bodies

    // Create some bodies to the left
    for (int i {0}; i < 22; i++){
        sim.addBody(
            {
                {-10000 + 200 * (rand() % 101), (rand() % 101) * 100}, {0, 0}, {0,0}, {2000}, 300
            } 
        );
    }

     sim.addBody(
            {
                {0 , 0}, {0, 0}, {0,0}, {100000}, 300
            } 
    );

    // create some bodies to the right
    for (int i {0}; i < 22; i++){
        sim.addBody(
            {
                {10000 + 200 * (rand() % 101), (rand() % 101) * 100}, {0, 0}, {0,0}, {2000}, 300
            } 
        );
    }

    // sim.addBody(
    //     {
    //         {0, 0}, {0, 0}, {100000}, 1
    //     }
       
    // );
    // sim.addBody(
    //      {
    //         {7000, 0}, {0, 100}, {2000}, 1
    //     } 
    // );
   

    renderer.start();
    while(true){
        renderer.render(sim.getBodies());

        sim.step();

        usleep(10000);

        keyEventHandler.handleKeyPress();
       
    }
    return 0;
}