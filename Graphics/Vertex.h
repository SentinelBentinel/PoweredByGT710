#pragma once

#include "../Math/Vector3.h"
#include "../Math/Color.h"

struct Vertex
{
    Vector3 position;
    Color color;

    inline static Vertex Lerp(const Vertex &a, const Vertex &b, float t)
    {
        Vertex result;

        result.position = a.position + (b.position - a.position) * t;

        result.color.r = static_cast<uint8_t>(a.color.r + (b.color.r - a.color.r) * t);
        result.color.g = static_cast<uint8_t>(a.color.g + (b.color.g - a.color.g) * t);
        result.color.b = static_cast<uint8_t>(a.color.b + (b.color.b - a.color.b) * t);

        return result;
    };
};
