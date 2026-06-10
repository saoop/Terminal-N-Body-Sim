#ifndef SIMULATION_M_H
#define SIMULATION_M_H

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

// Simulation
template <typename T> class Simulation {
  // static constexpr double G = 6.67430e-11; // m^3 kg^-1 s^-2
  static constexpr double G = 100; // m^3 kg^-1 s^-2, just for testing purposes.
private:
  double m_dt{1}; // time step, in seconds.

  std::vector<CircleBody<T>> m_bodies;

  bool m_paused{false};

  Vec2<T> computeGravity(T mass_1, T mass_2, Vec2<T> direction, T dist) {
    return direction * G * (mass_1 * mass_2) / (dist * dist);
  }

  Vec2<T> computeForce(int i, int j) {
    /* Computes force between bodies i and j */
    // Computes the force ON i
    if (i == j) {
      return Vec2<T>{0, 0};
    }

    Vec2<T> diff = m_bodies[j].getPos() -
                   m_bodies[i].getPos(); // force that j exerts on i.

    T dist{diff.norm()}; // TODO: still 0 division possible.

    T total_radius = m_bodies[i].getRadius() + m_bodies[j].getRadius();

    dist = std::max(dist, total_radius);

    Vec2<T> direction{diff / dist};

    Vec2<T> f{computeGravity(m_bodies[i].getMass(), m_bodies[j].getMass(),
                             direction, dist)};
    return f;
  }

public:
  void togglePause() { m_paused = !m_paused; }

  void step() {
    // Later parallelize
    if (m_paused) {
      return;
    }

    // Compute a force array.
    //

    std::vector<Vec2<T>> accelerations(m_bodies.size());
    for (std::size_t i{0}; i < m_bodies.size(); i++) {
      for (std::size_t j{i + 1}; j < m_bodies.size(); j++) {
        Vec2<T> force = computeForce(i, j);
        accelerations[i] += (force / m_bodies[i].getMass());
        accelerations[j] += (-force / m_bodies[j].getMass());
      }
    }

    for (int i{0}; i < m_bodies.size(); i++) {
      // xi+1 = xi + vi*dt + 1/2 ai*dt^2
      // vi+1 = vi + 1/2(ai + ai+1)*dt
      m_bodies[i].update(accelerations[i]);
    }
  }

  std::vector<CircleBody<T>> &getBodies() { return m_bodies; }

  void addBody(CircleBody<T> body) { m_bodies.push_back(body); }
};

#endif // SIMULATION_H