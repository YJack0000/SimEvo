#ifndef VEC2_HPP
#define VEC2_HPP

#include <cmath>
#include <utility>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x, float y) : x(x), y(y) {}

    // Allow implicit conversion to/from std::pair for backward compatibility
    Vec2(const std::pair<float, float>& p) : x(p.first), y(p.second) {}
    operator std::pair<float, float>() const { return {x, y}; }

    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

    float length() const { return std::sqrt(x * x + y * y); }

    Vec2 normalized(float maxLength) const {
        float len = length();
        if (len > maxLength && len > 0) {
            float scale = maxLength / len;
            return {x * scale, y * scale};
        }
        return *this;
    }

    bool isZero() const { return x == 0.0f && y == 0.0f; }
};

#endif
