#ifndef VEC2_HPP
#define VEC2_HPP

#include <cmath>
#include <utility>

/**
 * @brief A 2D vector struct used for positions and movement directions.
 *
 * Provides basic vector arithmetic, normalization, and implicit conversion
 * to/from std::pair<float, float> for backward compatibility with legacy APIs.
 */
struct Vec2 {
    float x = 0.0f;  ///< X component
    float y = 0.0f;  ///< Y component

    /// @brief Default constructor, initializes to (0, 0).
    Vec2() = default;

    /**
     * @brief Construct a Vec2 with given x and y components.
     * @param x The x component.
     * @param y The y component.
     */
    Vec2(float x, float y) : x(x), y(y) {}

    /// @brief Implicit conversion from std::pair for backward compatibility.
    Vec2(const std::pair<float, float>& p) : x(p.first), y(p.second) {}

    /// @brief Implicit conversion to std::pair for backward compatibility.
    operator std::pair<float, float>() const { return {x, y}; }

    /// @brief Component-wise addition.
    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }

    /// @brief Component-wise subtraction.
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }

    /// @brief Scalar multiplication.
    Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }

    /// @brief Compute the Euclidean length of the vector.
    float length() const { return std::sqrt(x * x + y * y); }

    /**
     * @brief Return a vector clamped to a maximum length.
     * @param maxLength The maximum allowed length.
     * @return A scaled-down copy if length exceeds maxLength, otherwise *this.
     */
    Vec2 normalized(float maxLength) const {
        float len = length();
        if (len > maxLength && len > 0) {
            float scale = maxLength / len;
            return {x * scale, y * scale};
        }
        return *this;
    }

    /// @brief Check whether both components are exactly zero.
    bool isZero() const { return x == 0.0f && y == 0.0f; }
};

#endif
