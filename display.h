
#ifndef DISPLAY_H
#define DISPLAY_H

#include <vector>
#include <iostream>
#include <termios.h> 
#include <unistd.h>
#include "math_utils.h"
#include "simulation.h"


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
    OTHER,
    NONE
};

Key readKey(){
    char buf[3];
    int n = read(STDIN_FILENO, buf, 3);
    if (n<= 0) return Key::NONE;
    switch (buf[2]){
        case 'A': return Key::UP;
        case 'B': return Key::DOWN;
        case 'C': return Key::RIGHT;
        case 'D': return Key::LEFT;
    }
    return Key::OTHER;
}

struct Grid {
    int m_height {};
    int m_width {};
    int m_size {};

    std::vector<bool> m_grid;

    Grid(int height, int width)
        : m_height{height}
        , m_width{width}
        , m_size {m_height * m_width}
        , m_grid(width * height, false)
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
    
    void set(int x, int y, bool val = true){
        if (!isValid(x, y)){
            return; // Silent, since it is just for rendering.
        }
        m_grid[y * m_width + x] = val;
    }

    bool is_set(int x, int y) const {
         if (!isValid(x, y)){
            return false; // Silent, since it is just for rendering.
        }
        return m_grid[y * m_width + x];
    }  

    bool operator()(int x, int y) const{
        return is_set(x, y);
    }
    
};


class Display {
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
    Display(int width, int height)
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
        if (new_size >= 10){
            m_pixel_size = new_size;
        }
    }

    void zoomIn() {
        setPixelSize(m_pixel_size * 10);
    }

    void ZoomOut() {
        setPixelSize(m_pixel_size / 10);
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


    void render(std::vector<CircleBody<double>>& bodies) const {
        std::cout << "------------------------------------------------------------------\n";

        // Grid of pixels.
        Grid grid {m_height, m_width};

        // std::vector<bool> grid(m_width * m_height, false); //
        // Check in what pixle each body lies.
        for(auto& body : bodies){
            auto const& pos {body.getPos()};
            
            int ind_y {pos.y / m_pixel_size + m_center_y - m_offset_y}; // Convert them from 0 centered to grid coords.
            int ind_x {pos.x / m_pixel_size + m_center_x - m_offset_x};

            grid.set(ind_x, ind_y);
        }
        

        // Drawing the bodies
        for (int y{0}; y < m_height;y++){
            for (int x{0};x<m_width;x++){
                // std::cout << grid(x, y);

                if (grid(x, y)){
                    std::cout << "*";
                }
                else {
                    std::cout << " ";
                }

                if (x == m_width - 1){
                    std::cout << "|";
                }
            }
            std::cout << "\n";

        }
        std::cout << "------------------------------------------------------------------\n";

        // grid.clear();

        clear();
    }
    

};

#endif // DISPLAY_H
