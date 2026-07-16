#pragma once

#include "../Math/Vector3.h"
#include "../Math/Color.h"
#include "../Math/Vector2.h"
#include <cstdint>

struct Vertex
{
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
    Color color;

    float brightness = 1.0f;
    float invW = 1.0f;

    inline static Vertex Lerp(const Vertex &a, const Vertex &b, float t)
    {
        Vertex result;

        result.position = a.position + (b.position - a.position) * t;
        result.normal = a.normal + (b.normal - a.normal) * t;
        result.uv = a.uv + (b.uv - a.uv) * t;

        result.color.r = static_cast<uint8_t>(a.color.r + (b.color.r - a.color.r) * t);
        result.color.g = static_cast<uint8_t>(a.color.g + (b.color.g - a.color.g) * t);
        result.color.b = static_cast<uint8_t>(a.color.b + (b.color.b - a.color.b) * t);

        return result;
    };
};
