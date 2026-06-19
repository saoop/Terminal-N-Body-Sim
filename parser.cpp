#include "parser.h"
#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

SimulationParams<double> loadTwoGalaxies() {
  double M = 10000;
  double R = 5000;
  double M_planet = M * 0.0001;
  double vel_offset = 10;
  double x_offset = 40000;
  double y_offset = 2000;
  constexpr double G = 100;

  SimulationParams<double> simulationParams{.name = "Two Galaxies"};
  simulationParams.bodies.push_back(BodyParams<double>{
      {-x_offset, -y_offset}, {vel_offset, 0}, {0, 0}, {M}, 400});
  simulationParams.bodies.push_back(BodyParams<double>{
      {x_offset, y_offset}, {-vel_offset, 0}, {0, 0}, {M}, 400});
  for (int i = 0; i < 5000; i++) {
    double rad = 2 * 3.14 * ((double)rand() / RAND_MAX);
    double r = R + 200 * (rand() % 100);
    double x = std::cos(rad);
    double y = std::sin(rad);
    double v = std::sqrt(G * M / r);
    double vel_x = -y;
    double vel_y = x;

    simulationParams.bodies.push_back(
        BodyParams<double>{{x_offset + r * x, y_offset + r * y},
                           {-vel_offset + v * vel_x, v * vel_y},
                           {0, 0},
                           {M_planet},
                           {100}});
    simulationParams.bodies.push_back(
        BodyParams<double>{{-x_offset + r * x, -y_offset + r * y},
                           {vel_offset + v * vel_x, v * vel_y},
                           {0, 0},
                           {M_planet},
                           {100}});
  }
  return simulationParams;
}

SimulationParams<double> parseYAML(const std::string &path) {
  YAML::Node yaml_file = YAML::LoadFile(path);
  auto scenario = yaml_file["scenario"].as<std::string>();
  if (scenario == "two_galaxies") {
    return loadTwoGalaxies();
  } else if (scenario.compare("newton")) {
    //
  } else if (scenario.compare("custom")) {
    //
  }
  return {};
}
