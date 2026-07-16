#pragma once

#include "../Math/Vector3.h"
#include "../Math/Color.h"

struct DirectionalLight
{
    Vector3 direction = {0.5f, -1.0f, -0.5f};
    Color color = {255,255,255};

    float intensity = 1.0f;
    float ambient = 0.15f;
};