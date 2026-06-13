#ifndef SIMULATION_M_H
#define SIMULATION_M_H
#include <memory>
#include <omp.h>
#include <stdexcept>

template <typename T = double> class Body {

protected:
  Vec2<T> m_pos{};
  Vec2<T> m_vel{};
  Vec2<T> m_acc{};
  T m_mass{};

public:
  Body(Vec2<T> pos, Vec2<T> vel, Vec2<T> acc, T mass)
      : m_pos{pos}, m_vel{vel}, m_acc{acc}, m_mass{mass} {}

  Vec2<T> const &getPos() const { return m_pos; }

  T const &getMass() const { return m_mass; }

  void update(Vec2<T> const &new_acc, double dt = 1) {
    // xi+1 = xi + vi*dt + 1/2 ai*dt^2
    // vi+1 = vi + 1/2(ai + ai+1)*dt
    updatePos(dt);
    updateVel(new_acc, dt);
    m_acc = new_acc;
  }

  void updatePos(double dt = 1) {
    m_pos += m_vel * dt + m_acc * 1 / 2 * dt * dt;
  }

  void setAcc(Vec2<T> acc) { m_acc = acc; }

  void updateVel(Vec2<T> const &new_acc, double dt = 1) {
    m_vel += (new_acc + m_acc) * 1 / 2 * dt;
  }
};

template <typename T = double> class CircleBody : public Body<T> {

private:
  T m_radius{};

public:
  CircleBody(Vec2<T> pos, Vec2<T> vel, Vec2<T> acc, T mass, T radius)
      : Body<T>(pos, vel, acc, mass), m_radius{radius} {}

  T getRadius() { return m_radius; }
};

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