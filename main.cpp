#include <iostream>
#include <unistd.h>
#include <vector>

#include "app.h"
#include <cmath>
#include <memory>

#include "states/states.h"
#include <sys/ioctl.h>

int main() {
  constexpr double SUN_MASS = 1.989e30;
  constexpr double EARTH_MASS = 5.972e24;
  constexpr double EARTH_SUN_DIST = 1.496e11;     // 1 AU in meters
  constexpr double EARTH_ORBITAL_VEL = 29783.0;   // m/s
  constexpr double JUPITER_SUN_DIST = 7.785e11;   // ~5.2 AU
  constexpr double JUPITER_ORBITAL_VEL = 13070.0; // m/s
  constexpr double JUPITER_MASS = 1.898e27;
  constexpr double MOON_MASS = 7.342e22;
  constexpr double MOON_EARTH_DIST = 3.844e8;
  constexpr double MOON_ORBITAL_VEL = 1022.0; // m/s relative to Earth
  // constexpr double G = 6.674e-11;             // N⋅m²/kg²
  constexpr double G = 100;

  std::cout << "\033[2J"; // clear entire screen once
  std::cout << "\033[H";  // move to top left once
  std::cout << "\033[s";  // save this as our origin

  App app{};
  // std::unique_ptr<MenuState> menu = ;
  app.setState(std::make_unique<MenuState>(app));
  app.run();

  return 0;
}