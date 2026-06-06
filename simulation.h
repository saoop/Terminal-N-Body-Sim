#ifndef SIMULATION_H
#define SIMULATION_H

template <typename T=double>
class Body {

protected:
    Vec2<T> m_pos {};
    Vec2<T> m_vel {};
    T m_mass {};

public:
    Body (Vec2<T> pos, Vec2<T> vel, T mass) 
    : m_pos{pos}
    , m_vel{vel}
    , m_mass{mass}
    {}

    Vec2<T> const& getPos () const{
        return m_pos;
    }

    T const& getMass() const {
        return m_mass;
    }

    void updatePos(){
        m_pos += m_vel;
    }

    void updateVel(Vec2<T> const& acceleration){
        m_vel += acceleration;
    }
};


template <typename T=double>
class CircleBody : public Body<T>
{

private:
    T m_radius {};
public:
    CircleBody(Vec2<T> pos, Vec2<T> vel, T mass, T radius)
    : Body<T> {pos, vel, mass}
    , m_radius{radius} {
        std::cout << "Initialized Circle" << '\n';
    }
};



// Simulation
template <typename T>
class Simulation {
    // static constexpr double G = 6.67430e-11; // m^3 kg^-1 s^-2
    static constexpr double G = 100; // m^3 kg^-1 s^-2, just for testing purposes.
    private:

    std::vector<CircleBody<T>> m_bodies;

    bool m_paused{false};

    // For now a dumb solution.
    Vec2<T> computeAcceleration(int index){ // TODO use a reference instead
        Vec2<T> total_acc {0,0};

        // Newton formula
        for (std::size_t i{0}; i < m_bodies.size(); i++){
            if (i == index){
                continue;
            }
            
            Vec2<T> diff  = m_bodies[i].getPos() - m_bodies[index].getPos();

            T dist { diff.norm()};

            dist= std::max(dist, 0.0001); // clip for very small values
            //TODO: replace with the radius...

            Vec2<T> direction {diff / dist}; 


            Vec2<T> acc {direction * G * (m_bodies[i].getMass()) / (dist * dist)};
            total_acc += acc;
        }
        return total_acc;
    }

    public:

    void togglePause(){
        m_paused = !m_paused;
    }

    void step(){
        // Later parallelize
        if(m_paused){
            return;
        }

        std::vector<Vec2<T>> accelerations;
        for (std::size_t i {0}; i < m_bodies.size(); i++){
            // Vec2<T> n_acc = ;
            // std::cout << "acceleration: " << n_acc.x << " " << n_acc.y << '\n';
            accelerations.push_back(computeAcceleration(i)); 
        }

        for (int i {0}; i < m_bodies.size(); i++){
            m_bodies[i].updatePos();

            m_bodies[i].updateVel(accelerations[i]);
        }   
    }

    std::vector<CircleBody<T>>& getBodies(){
        return m_bodies;
    }

    void addBody(CircleBody<T> body){
        m_bodies.push_back(body);
    }
};


#endif // SIMULATION_H