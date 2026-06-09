#include <iostream>
#include <vector>
#include <unistd.h>

#include "math_utils.h"
#include "simulation.h"
#include "renderer.h"
#include <unordered_map>
#include <memory>
#include "commands.h"
#include <bits/chrono.h>


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
    Menu menu{};

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
    for (int i {0}; i < 42; i++){
        sim.addBody(
            {
                {-10000 + 200 * (rand() % 101), (rand() % 101) * 100}, {0, 0}, {0,0}, {2000}, 100
            } 
        );
    }

     sim.addBody(
            {
                {0 , 0}, {0, 0}, {0,0}, {100000}, 200
            } 
    );

    // create some bodies to the right
    for (int i {0}; i < 42; i++){
        sim.addBody(
            {
                {10000 + 200 * (rand() % 101), (rand() % 101) * 100}, {0, 0}, {0,0}, {2000}, 100
            } 
        );
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
   

    int max_fps {200}; 

    int frame_length {1000000 / max_fps}; // in microseconds

    int minimal_wait {10000};
    int current_fps {0};


    renderer.start();
    while(true){
        // TODO: write a timer.
        auto start = std::chrono::steady_clock::now();
        menu.render(current_fps, sim.getBodies().size());
        renderer.render(sim.getBodies());
        menu.clear();
        
        sim.step();

        auto stop = std::chrono::steady_clock::now();
        int duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
        
        auto to_sleep {std::max(0, minimal_wait - duration)};
        current_fps = 1000000/ (to_sleep + duration); // to Seconds conversion

        usleep(to_sleep);

        // std::cout << "FPS: " << current_fps;

        keyEventHandler.handleKeyPress();
       
    }
    return 0;
}