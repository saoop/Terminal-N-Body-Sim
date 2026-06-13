#ifndef SIMULATION_M_H
#define SIMULATION_M_H
#include "bodies.h"
#include "force_computers.h"
#include <memory>
#include <omp.h>
#include <stdexcept>

// Simulation
template <typename T> class Simulation {
private:
  double m_dt{1}; // time step, in seconds.
  std::unique_ptr<ForcesComputer<T>> m_forcesComputer;

  std::vector<CircleBody<T>> m_bodies;

  bool m_paused{false};

public:
  Simulation(std::unique_ptr<ForcesComputer<T>> forcesComputer, double dt = 1)
      : m_forcesComputer{std::move(forcesComputer)} {
    if (dt < 0) {
      throw std::runtime_error("dt cannot be less than 0!");
    }

    m_dt = dt;
  }

  void togglePause() { m_paused = !m_paused; }

  void step() {
    // Later parallelize
    if (m_paused) {
      return;
    }

    // Compute the force array.
    std::vector<Vec2<T>> forces(m_bodies.size() * m_bodies.size());
    m_forcesComputer->computeForces(forces, m_bodies);

    // Apply forces to compute new accelerations for each body
    std::vector<Vec2<T>> accelerations(m_bodies.size());

    for (std::size_t i{0}; i < m_bodies.size(); i++) {
      Vec2<T> sum{};
#pragma omp parallel for reduction(vec2_plus : sum)
      for (std::size_t j = i + 1; j < m_bodies.size(); j++)

      {
        sum += (forces.at(i * m_bodies.size() + j) / m_bodies[i].getMass());
        accelerations[j] +=
            (-forces.at(i * m_bodies.size() + j) / m_bodies[j].getMass());
      }

      accelerations[i] += sum;
    }

    // Apply the accelerations.

#pragma omp parallel for
    for (int i = 0; i < m_bodies.size(); i++) {
      m_bodies[i].update(accelerations[i], m_dt);
    }
  }

  std::vector<CircleBody<T>> &getBodies() { return m_bodies; }

  void addBody(CircleBody<T> body) { m_bodies.push_back(body); }
};

#endif // SIMULATION_H