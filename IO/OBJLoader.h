#pragma once

#include <string>

#include "../Graphics/Mesh.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Graphics/Vertex.h"

class OBJLoader
{
public:
    static Mesh Load(const std::string &path);
};
