#pragma once

#include <vector>

#include "Vertex.h"
#include "Transform.h"
#include "Texture.h"
#include "Material.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::vector<Material> materials;
    std::vector<int> materialIndices;

    Transform transform;
    Texture texture;
    bool hasTexture = false;
};