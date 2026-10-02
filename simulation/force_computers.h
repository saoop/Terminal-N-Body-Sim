#ifndef FORCE_COMPUTERS_H
#define FORCE_COMPUTERS_H

#include "../common/math_utils.h"
#include "../simulation/bodies.h"
#include "utils.h"
#include <limits>
#include <memory>
#include <queue>
#include <stdio.h>
#include <vector>

template <typename T> class ForcesComputer {
protected:
  double m_G{};
  Vec2<T> computeGravity(T mass_1, T mass_2, Vec2<T> direction, T dist) {
    return direction * m_G * (mass_1 * mass_2) / (dist * dist);
  }

  T computePotentialEnergy(T mass_1, T mass_2, T dist) {
    return -0.5 * m_G * mass_1 * mass_2 / dist;
  }

public:
  ForcesComputer(double G) : m_G{G} {}
  virtual void computeForces(std::vector<Vec2<T>> &forces,
                             std::vector<T> &energies,
                             std::vector<CircleBody<T>> &bodies) = 0;
};

template <typename T>
class BruteForceForcesComputer : public ForcesComputer<T> {

public:
  BruteForceForcesComputer(double G) : ForcesComputer<T>{G} {}

  T computePotential(CircleBody<T> &body_i, CircleBody<T> &body_j) {
    if (&body_i == &body_j) {
      return 0;
    }

    T dist{body_i.getDist(body_j)};

    return this->computePotentialEnergy(body_i.getMass(), body_j.getMass(),
                                        dist);
  }

  std::pair<T, Vec2<T>> energyAndForce(CircleBody<T> &body_i,
                                       CircleBody<T> &body_j) {
    if (&body_i == &body_j) {
      return {0, Vec2<T>{0, 0}};
    }
    T dist{body_i.getDist(body_j)};

    T energy{
        this->computePotentialEnergy(body_i.getMass(), body_j.getMass(), dist)};

    Vec2<T> diff =
        body_j.getPos() - body_i.getPos(); // force that j exerts on i.

    Vec2<T> direction{diff / dist};

    Vec2<T> f{this->computeGravity(body_i.getMass(), body_j.getMass(),
                                   direction, dist)};

    return {energy, f};
  }

  Vec2<T> computeForce(CircleBody<T> &body_i, CircleBody<T> &body_j) {
    /* Computes force between bodies i and j */
    // Computes the force ON i
    if (&body_i == &body_j) {
      return Vec2<T>{0, 0};
    }

    Vec2<T> diff =
        body_j.getPos() - body_i.getPos(); // force that j exerts on i.

    T dist{diff.norm()};

    T total_radius = std::max(body_i.getRadius() + body_j.getRadius(), 0.001);

    dist = std::max(dist, total_radius);

    Vec2<T> direction{diff / dist};

    Vec2<T> f{this->computeGravity(body_i.getMass(), body_j.getMass(),
                                   direction, dist)};
    return f;
  }

  void computeForces(std::vector<Vec2<T>> &forces, std::vector<T> &energies,

                     std::vector<CircleBody<T>> &bodies) override {
    // allocate the 2d grid of forces

#pragma omp parallel for
    for (std::size_t i = 0; i < bodies.size(); i++) {
      Vec2<T> sum{};
      T energy_sum{0};
#pragma omp parallel for reduction(vec2_plus : sum) reduction(+ : energy_sum)
      for (std::size_t j = 0; j < bodies.size(); j++) {
        auto [energy, force] = energyAndForce(bodies[i], bodies[j]);
        // forces.at(i * bodies.size() + j) = force;
        sum += force;
        energy_sum += energy;
      }
      forces.at(i) = sum;
      energies.at(i) = energy_sum;
    }
  }
};

template <typename T>
class BarnesHutsForcesComputer : public ForcesComputer<T> {
private:
  double m_theta;

public:
  BarnesHutsForcesComputer(double G, double theta = 0.5)
      : ForcesComputer<T>{G}, m_theta{theta} {}

  void computeForces(std::vector<Vec2<T>> &forces, std::vector<T> &energies,
                     std::vector<CircleBody<T>> &bodies) override {
    // Find the bounding box of the whole system.
    std::pair<Vec2<T>, Vec2<T>> corners{findCorners(bodies)};
    Vec2<T> diff = corners.second - corners.first;
    T width = std::max(diff.x, diff.y);

    // Build the tree
    QuadTree<T> tree{corners.first, width};
    for (auto &body : bodies) {
      tree.insert(body.getPos(), body.getMass());
    }

// Calculate forces on each body
#pragma omp parallel for
    for (int i = 0; i < bodies.size(); i++) {
      // traverse the tree.
      auto [e, field] = tree.traverse(bodies[i].getPos(), m_theta);
      Vec2<T> force{field * (this->m_G * bodies[i].getMass())};
      forces.at(i) += force;
      energies.at(i) += (-this->m_G * 0.5 * e * bodies[i].getMass());
    }
  }
};

#endif