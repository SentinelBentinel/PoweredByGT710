#pragma once

#include <string>
#include "../Math/Color.h"
#include "Texture.h"

struct Material
{
    std::string name;

    Color ambient = {32,32,32};
    Color diffuse = {255,255,255};
    Color specular = {255,255,255};
    float shininess = 32.0f;
    float opacity = 1.0f;

    bool hasDiffuseMap = false;
    Texture diffuseMap;
};
