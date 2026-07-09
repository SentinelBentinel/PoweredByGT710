#pragma once

#include "../Math/Vector3.h"

struct Transform
{
    Vector3 position;
    Vector3 rotation;
    Vector3 scale = {1,1,1};
};
