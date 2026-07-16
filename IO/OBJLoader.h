#pragma once

#include <string>

#include "../Graphics/Mesh.h"

class OBJLoader
{
public:
    static Mesh Load(const std::string &path);
};