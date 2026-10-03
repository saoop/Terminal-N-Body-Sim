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
  T m_current_energy;
  bool m_energy_valid{false}; // false until energy was computed for the
                              // current bodies
  bool m_paused{false};

  // Total energy: kinetic + potential (energies = per-body potential)
  T sumEnergy(std::vector<T> const &energies) {
    T total_energy = 0;

#pragma omp parallel for reduction(+ : total_energy)
    for (int i = 0; i < m_bodies.size(); i++) {

      T kinetic_energy = 0.5 * (m_bodies[i].getMass() *
                                (std::pow(m_bodies[i].getVelScalar(), 2)));
      total_energy += (kinetic_energy + energies[i]);
    }
    return total_energy;
  }

public:
  Simulation(std::unique_ptr<ForcesComputer<T>> forcesComputer, double dt = 1)
      : m_forcesComputer{std::move(forcesComputer)} {
    if (dt < 0) {
      throw std::runtime_error("dt cannot be less than 0!");
    }
    m_current_energy = 0;
    m_dt = dt;
  }

  void togglePause() { m_paused = !m_paused; }

  void step() {
    // Later parallelize
    if (m_paused) {
      return;
    }

    // Compute the force array.
    std::vector<Vec2<T>> forces(m_bodies.size());
    std::vector<T> energies(m_bodies.size());
    m_forcesComputer->computeForces(forces, energies, m_bodies);

    // Energy calculataion
    m_current_energy = sumEnergy(energies);
    m_energy_valid = true;
    // Apply forces to compute new accelerations for each body
    std::vector<Vec2<T>> accelerations(m_bodies.size());

#pragma omp parallel for
    for (std::size_t i = 0; i < m_bodies.size(); i++) {
      accelerations[i] = forces.at(i) / m_bodies[i].getMass();
    }

    // Apply the accelerations.

#pragma omp parallel for
    for (int i = 0; i < m_bodies.size(); i++) {
      m_bodies[i].update(accelerations[i], m_dt);
    }
  }

  std::vector<CircleBody<T>> &getBodies() { return m_bodies; }
  T getTotalEnergy() {
    // Before the first step there is no energy yet, compute it once here
    // (otherwise the first frame would show 0).
    if (!m_energy_valid) {
      std::vector<Vec2<T>> forces(m_bodies.size());
      std::vector<T> energies(m_bodies.size());
      m_forcesComputer->computeForces(forces, energies, m_bodies);
      m_current_energy = sumEnergy(energies);
      m_energy_valid = true;
    }
    return m_current_energy;
  }

  void addBody(CircleBody<T> body) {
    m_bodies.push_back(body);
    m_energy_valid = false;
  }
};

#endif // SIMULATION_H