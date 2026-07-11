#pragma once

struct Stats
{
    float fps = 0.0f;
    float frameTime = 0.0f;

    int vertices = 0;
    int meshes = 0;
    int triangles = 0;
    int trianglesCulled = 0;
    int pixelsDrawn = 0;
};