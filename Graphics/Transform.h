#pragma once

#include "../Math/Vector2.h"

struct Transform
{
    Vector2 position;
    float rotation = 0.0f;
    float scale = 1.0f;
};
