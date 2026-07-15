#pragma once

#include <vector>

#include "Vertex.h"
#include "Transform.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    Transform transform;
};