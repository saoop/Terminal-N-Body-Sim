
#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include <iostream>
#include <termios.h> 
#include <unistd.h>
#include "math_utils.h"
#include "simulation.h"
#include <map>


void enableRawMode(termios& original) {
    termios raw;
    tcgetattr(STDIN_FILENO, &original);  // save original
    raw = original;
    raw.c_lflag &= ~(ICANON | ECHO);    // disable buffering and echo
    raw.c_cc[VMIN] = 0; // non-blocking
    raw.c_cc[VTIME] = 0;                 // no timeout
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);                                                                                                                                                                                     
}

void disableRawMode(termios& original) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}


struct RawMode {
    termios original;
    RawMode()  { enableRawMode(original); }
    ~RawMode() { disableRawMode(original); }
};

enum class Key{
    UP,
    DOWN,
    RIGHT,
    LEFT,
    SPACE,
    ZOOM_IN,
    ZOOM_OUT,
    OTHER,
    NONE
};

Key readKey(){
    char buf[3];
    int n = read(STDIN_FILENO, buf, 3);

    if (n<= 0) return Key::NONE;

    // Check the first one in the buffer
    switch (buf[0]){
        case ' ': return Key::SPACE;
        case '-': return Key::ZOOM_OUT;// Using just + and - here, because Ctrl is handled diffferently on different terminals.`
        case '+': return Key::ZOOM_IN;
    }
    
    switch (buf[2]){
        case 'A': return Key::UP;
        case 'B': return Key::DOWN;
        case 'C': return Key::RIGHT;
        case 'D': return Key::LEFT;
    }
    return Key::OTHER;
}

// enum class 

namespace Pixels{
    inline const std::map<int, char> pixel_map {
        {0, ' '},
        {1, '.'},
        {2, '*'},
        {3, 'o'},
        {4, '0'},
        {5, '@'}
    };
}

struct Grid {
    int m_height {};
    int m_width {};
    int m_size {};

    std::vector<int> m_grid;

    Grid(int height, int width)
        : m_height{height}
        , m_width{width}
        , m_size {m_height * m_width}
        , m_grid(width * height, 0)
    {
        
    }

    bool isValid(int x, int y) const {
        return y < m_height && x < m_width && y >= 0 && x >=0;
    }

    void clear(){
        for (size_t i{0}; i < m_size; i ++){
            m_grid[i] = false;
        }
    }
    
    void set(int x, int y, int val = 1){
        if (!isValid(x, y)){
            return; // Silent, since it is just for rendering.
        }
        m_grid[y * m_width + x] = val;
    }

    int operator()(int x, int y) const{
        if (!isValid(x, y)){
            return false; // Silent, since it is just for rendering.
        }
        return m_grid[y * m_width + x];
    }
    
};


class Renderer {
    /*
    This class defines the environment variables such as pixel size, zoom etc.
    */
private:
    int m_width {};
    int m_height {};
    int m_center_x {};
    int m_center_y {};
    double m_pixel_size {1000}; // in meters???
    int m_offset_x {0};
    int m_offset_y {0};

public:
    Renderer(int width, int height)
    : m_width{width}
    , m_height{height}
    {
        std::cout << "Initializing the Display..." << '\n';
        std::cout << std::endl;

        m_center_x = m_width / 2;
        m_center_y = m_height / 2;
    }
    
    void nudge(int offset_x, int offset_y){
        m_offset_x += offset_x;
        m_offset_y += offset_y;
    }

    double getPixelSize() const {
        return m_pixel_size;
    }

    void setPixelSize(double new_size){
        // if (new_size >= 10){
        m_pixel_size = new_size;
        // }
    }

    void zoom(double factor){
        setPixelSize(m_pixel_size * factor);
    }

    void start(){
    }

    void clear() const{
        for (int i {0}; i < m_height; i++){
            std::cout << "\033[A\r";
        }

        // For the border??
        std::cout << "\033[A\r";
        std::cout << "\033[A\r";
    
    }

    void drawPixel(std::string& s) const{
        std::cout << s;
    }

    void drawBorder() const {
        for (int i {0}; i < m_width + 1; i++){
            std::cout << "-";
        }
        std::cout << "\n";
    }

    void render(std::vector<CircleBody<double>>& bodies) const {
        drawBorder();

        // Grid of pixels.
        Grid grid {m_height, m_width};

        // std::vector<bool> grid(m_width * m_height, false); //
        // Check in what pixle each body lies.
        for(auto& body : bodies){

            //TODO: optimization: should skip all bodies that do not fit into the window.


            auto const& pos {body.getPos()};
            
            int ind_y {pos.y / m_pixel_size + m_center_y - m_offset_y}; // Convert them from 0 centered to grid coords.
            int ind_x {pos.x / m_pixel_size + m_center_x - m_offset_x};
            
            double radius = body.getRadius();

            double diameter = radius * 2;

            int radius_in_pixels = static_cast<int>(radius / m_pixel_size);

            //Check if some part of the body is visible
            if(ind_x + radius_in_pixels < 0 || ind_x - radius_in_pixels >= m_width || ind_y + radius_in_pixels < 0 || ind_y - radius_in_pixels >= m_height){
                continue;
            }

            
            if (diameter <= m_pixel_size){
                int times = static_cast<int>(diameter / m_pixel_size);

                times = std::min(times, 5);
                times = std::max(times, 1);

                grid.set(ind_x, ind_y, times);
            } 
            else {
                grid.set(ind_x, ind_y, 5);
                // Draw a circle

                // IDK how to do it efficiently...
                double r_squared = radius_in_pixels * radius_in_pixels;

                for (int i=0; i< radius_in_pixels * 2 + 1; i ++){
                    for (int j = 0; j < radius_in_pixels * 2 + 1; j++){
                        int x { i - radius_in_pixels };
                        int y { j - radius_in_pixels};

                        // Can use map 0.01, 0.04, 0.09 ... -> squares of usual 0.1, 0.2, 0.3, etc...
                        // To render the intensity of the pixel (center -> stronger)

                        if (x * x + y * y <= r_squared){
                            grid.set(ind_x + x, ind_y + y, 5);
                        }
                    }
                }
            }
        }
        

        // Drawing the bodies
        for (int y{0}; y < m_height;y++){
            for (int x{0};x<m_width;x++){
                std::cout << Pixels::pixel_map.at(grid(x,y));


                if (x == m_width - 1){
                    std::cout << "|";
                }
            }
            std::cout << "\n";

        }
        drawBorder();
        clear();
    }
    

};

#endif // DISPLAY_H
