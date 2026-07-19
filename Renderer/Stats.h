#pragma once

#include <atomic>

struct Stats
{
    float fps = 0.0f;
    float frameTime = 0.0f;

    int vertices = 0;
    int meshes = 0;
    int triangles = 0;
    int trianglesCulled = 0;
    std::atomic<int> pixelsDrawn = 0;
    int hardwareThreads = 0;
    int activeThreads = 0;
    int logicalCores = 0;
};