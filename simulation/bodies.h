#ifndef BODIES_H
#define BODIES_H
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

  // TODO: Euler + Verlet
  void update(Vec2<T> const &new_acc, double dt = 1) {
    // xi+1 = xi + vi*dt + 1/2 ai*dt^2
    // vi+1 = vi + 1/2(ai + ai+1)*dt
    updateVel(new_acc, dt);
    m_acc = new_acc;
    updatePos(dt);
  }

  void updatePos(double dt = 1) {
    m_pos += m_vel * dt + m_acc * 1 / 2 * dt * dt;
  }

  void setAcc(Vec2<T> acc) { m_acc = acc; }

  void updateVel(Vec2<T> const &new_acc, double dt = 1) {
    m_vel += (new_acc + m_acc) * 1 / 2 * dt;
  }

  T getVelScalar() { return m_vel.norm(); }
};

template <typename T = double> class CircleBody : public Body<T> {

private:
  T m_radius{};

public:
  CircleBody(Vec2<T> pos, Vec2<T> vel, Vec2<T> acc, T mass, T radius)
      : Body<T>(pos, vel, acc, mass), m_radius{radius} {}

  T getRadius() { return m_radius; }
  T getDist(CircleBody<T> &other) {
    Vec2<T> diff = this->getPos() - other.getPos();

    T dist{diff.norm()};
    T total_radius = std::max(this->getRadius() + other.getRadius(), 0.001);

    return std::max(dist, total_radius);
  }
};

#endif