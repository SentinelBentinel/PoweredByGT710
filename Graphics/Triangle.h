#pragma once

#include "Vertex.h"
#include "Transform.h"

struct Triangle
{
    Vertex v0;
    Vertex v1;
    Vertex v2;

    Transform transform;
};