#pragma once

#include <cmath>

struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr Vector2(float x, float y)
        : x(x), y(y) {}

    constexpr Vector2 operator+(const Vector2& other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vector2 operator-(const Vector2& other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr Vector2 operator*(float scalar) const
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vector2 operator/(float scalar) const
    {
        return {x / scalar, y / scalar};
    }

    Vector2& operator+=(const Vector2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    float Length() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vector2 Normalized() const
    {
        float length = Length();

        if (length == 0.0f)
            return {};

        return *this / length;
    }

    static float Dot(const Vector2& a, const Vector2& b)
    {
        return a.x * b.x + a.y * b.y;
    }
};