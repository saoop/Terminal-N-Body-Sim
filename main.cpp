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

class Timer{
    private:
        std::chrono::_V2::steady_clock::time_point m_start;
        std::chrono::_V2::steady_clock::time_point m_end;

    public:
    std::chrono::_V2::steady_clock::time_point start(){
        m_start = std::chrono::steady_clock::now();
        return m_start;
    }

    std::chrono::_V2::steady_clock::time_point stop(){
        m_end = std::chrono::steady_clock::now();
        return m_end;
    }

    int elaspsed() const{
        if (m_start > m_end){
            return -1;
        }
        return std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count();
    }
};

class FPSController {
    private:
    int m_max_fps {}; // in 1/Seconds
    int m_minimal_wait {}; // In Microseconds
    int m_max_frame_length {}; // In Microseconds
    int m_current_fps {};
    Timer m_timer {};
    
    public:
    FPSController(int max_fps, int minimal_wait)
    : m_max_fps{max_fps}
    , m_minimal_wait{minimal_wait}
    , m_max_frame_length {1000000 / max_fps}
    {

    }

    int getCurrentFPS() const{
        return m_current_fps;
    }

    void startFrame(){
        m_timer.start();
    }

    void endFrame(){
        m_timer.stop();

        auto to_sleep {std::max(0, m_minimal_wait - m_timer.elaspsed())};

        // Update the fps
        m_current_fps = 1000000/ (to_sleep + m_timer.elaspsed()); // to Seconds conversion

        //Sleep the rest of the frame
        usleep(to_sleep);

    }
};

int main() {
    RawMode raw_mode; // RAII for raw mode
    Simulation<double> sim {};
    Renderer renderer {125, 35};
    Menu menu{};
    // Timer timer {};

    FPSController fpsController {100, 10000};

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
        fpsController.startFrame();

        menu.render(fpsController.getCurrentFPS(), sim.getBodies().size());
        renderer.render(sim.getBodies());
        menu.clear();
        
        sim.step();

        fpsController.endFrame();
        
        keyEventHandler.handleKeyPress();
       
    }
    return 0;
}