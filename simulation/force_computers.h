#ifndef FORCE_COMPUTERS_H
#define FORCE_COMPUTERS_H

#include "../math_utils.h"
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

public:
  ForcesComputer(double G) : m_G{G} {}
  virtual void computeForces(std::vector<Vec2<T>> &forces,
                             std::vector<CircleBody<T>> &bodies) = 0;
};

template <typename T>
class BruteForceForcesComputer : public ForcesComputer<T> {

public:
  BruteForceForcesComputer(double G) : ForcesComputer<T>{G} {}
  Vec2<T> computeForce(CircleBody<T> &body_i, CircleBody<T> &body_j) {
    /* Computes force between bodies i and j */
    // Computes the force ON i
    if (&body_i == &body_j) {
      return Vec2<T>{0, 0};
    }

    Vec2<T> diff =
        body_j.getPos() - body_i.getPos(); // force that j exerts on i.

    T dist{diff.norm()}; // TODO: still 0 division possible.

    T total_radius = body_i.getRadius() + body_j.getRadius();

    dist = std::max(dist, total_radius);

    Vec2<T> direction{diff / dist};

    Vec2<T> f{this->computeGravity(body_i.getMass(), body_j.getMass(),
                                   direction, dist)};
    return f;
  }

  void computeForces(std::vector<Vec2<T>> &forces,
                     std::vector<CircleBody<T>> &bodies) override {
#pragma omp parallel for
    for (std::size_t i = 0; i < bodies.size(); i++) {
#pragma omp parallel for
      for (std::size_t j = i + 1; j < bodies.size(); j++) {
        Vec2<T> force = computeForce(bodies[i], bodies[j]);
        forces.at(i * bodies.size() + j) = force;
      }
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

  void computeForces(std::vector<Vec2<T>> &forces,
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
      Vec2<T> field{tree.traverse(bodies[i].getPos(), m_theta)};
      Vec2<T> force{field * (this->m_G * bodies[i].getMass())};
      forces.at(i) = force;
    }
  }
};

#endif