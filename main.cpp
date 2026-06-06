#include <iostream>
#include <vector>
#include "math_utils.h"
#include "simulation.h"
#include "display.h"


//2D grid.

// .-*-.
// *****
// ''*''

// .*.
// ***
// '*'

//  *
// ***
//  *

//  .
// -*-
//  '

// *

// .

//  ■■
// ■■■■
//  ■■

//  ▪▪▪▪
// ▪▪▪▪▪▪
//  ▪▪▪▪


// Grid ranges from 1 - 8 with 8 being the biggest character like ■ small ▪

// Want to make scrolling possible

// All bodies  are circles.
// As input we provide center and radius
// We also provide weight (for gravity)


// Have a scale parameter ? Then we coul use scrolling to zoom in and out.
// Should the scrolling coef be the same? Should we for example always multiply by 2 when scrolling?

// How to calculate where the particle is located on the grid terminal? So we could say something like the following:
// pixel size = 10m
// we just calculate the distance between 2 objects, distance / pix_size = the number of pixels the object should be displayed at.
// 

//Set the center at 0 0
//Width and height only as display.
// encode each pixel with some distance.
// when checking just look at 

//Should we have a class for display?

// Bodies




int main() {
    RawMode raw_mode; // RAII for raw mode
    Simulation<double> sim {};
    Display display {71, 31};

    sim.addBody(
        {
            {0, 0}, {0, 0}, {100000}, 1
        }
       
    );
    sim.addBody(
         {
            {7000, 0}, {0, 100}, {2000}, 1
        }
    );
   

    display.start();
    while(true){
        display.render(sim.getBodies());

        sim.step();

        usleep(10000);

        Key key = readKey();
        switch (key){
            case Key::UP:
                display.nudge(0, -1);
                break;
            case Key::DOWN:
                display.nudge(0, 1);
                break;
            case Key::RIGHT:
                display.nudge(1, 0);
                break;
            case Key::LEFT:
                display.nudge(-1, 0);
                break;
            default:
                break;
        }
    }
    // disableRawMode(original);
    return 0;
}