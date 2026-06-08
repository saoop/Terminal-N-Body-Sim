#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>
template <typename T=double>
struct Vec2 {
    T x {}, y {};

    Vec2() = default; // Default constructor, initializes to (0,0)
    Vec2 (T x, T y): x{x}, y{y} {} // Just copy the x and y.

    Vec2 operator+(Vec2 const& other) const{
        return {x + other.x, y + other.y};
    }

    Vec2& add(Vec2 const& other){
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator+=(Vec2 const& other){
        return add(other);
    }

    Vec2 operator*(T scalar) const{
        return {x * scalar, y * scalar};
    }

    Vec2 operator-(Vec2 const& other) const{
        return {x - other.x, y - other.y};
    }

    Vec2 operator-() const {
        return {-x, -y};
    }
    
    Vec2& mul(T scalar){
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2& operator*=(T scalar){
        return mul(scalar);
    }

    Vec2 operator/(T scalar) const{
        return {x / scalar, y / scalar};
    }

    T norm() const{
        return static_cast<T>(std::sqrt(x*x + y*y));
    }
};


#endif // MATH_UTILS_H