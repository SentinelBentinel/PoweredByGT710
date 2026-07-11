#include "Renderer/Renderer.h"
#include "Graphics/Triangle.h"
#include "Graphics/Mesh.h"
#include "Core/Timer.h"

#include <vector>
#include <iostream>

Mesh cube =
    {
        // Vertices
        {
            {{-100, -100, -100}, {255, 0, 0}},  // 0
            {{100, -100, -100}, {0, 255, 0}},   // 1
            {{100, 100, -100}, {0, 0, 255}},    // 2
            {{-100, 100, -100}, {255, 255, 0}}, // 3

            {{-100, -100, 100}, {255, 0, 255}}, // 4
            {{100, -100, 100}, {0, 255, 255}},  // 5
            {{100, 100, 100}, {255, 255, 255}}, // 6
            {{-100, 100, 100}, {255, 128, 0}}   // 7
        },

        // Indices (12 triangles)
        {
            // Front (+Z)
            4, 5, 6,
            4, 6, 7,

            // Back (-Z)
            0, 2, 1,
            0, 3, 2,

            // Left (-X)
            0, 4, 7,
            0, 7, 3,

            // Right (+X)
            1, 2, 6,
            1, 6, 5,

            // Top (+Y)
            3, 7, 6,
            3, 6, 2,

            // Bottom (-Y)
            0, 1, 5,
            0, 5, 4},

        // Transform
        {
            {0.0f, 0.0f, 500.0f}, // Position
            {0.0f, 0.0f, 0.0f},   // Rotation (XYZ)
            {1.0f, 1.0f, 1.0f}    // Scale
        }};
Mesh frontCube = []()
{
    Mesh m = cube;
    m.transform.position = {0.0f, 0.0f, 600.0f};
    m.transform.scale = {2.0f,2.0f,2.0f};
    for (auto &v : m.vertices)
        v.color = {255, 26, 43};
    return m;
}();
Mesh backCube = []()
{
    Mesh mb = cube;
    mb.transform.position = {0.0f, 0.0f, 500.0f};
    for (auto &v : mb.vertices)
        v.color = {12, 255, 87};
    return mb;
}();

std::vector<Mesh> meshes =
    {
        backCube, frontCube};

void Update(const Timer &timer)
{
    (void)timer;
    meshes[0].transform.rotation.x += 0.001f;
    meshes[0].transform.rotation.y += 0.001f;
    meshes[1].transform.rotation.x += 0.001f;
    meshes[1].transform.rotation.y += 0.001f;
}

void Render(Renderer &renderer)
{
    renderer.Clear({30, 30, 30});

    for (const Mesh &mesh : meshes)
    {
        renderer.RenderMesh(mesh);
    }

    renderer.Present();
}

int main()
{
    std::cout << "Hardware acceleration sold separately.\n";
    Renderer renderer(800, 600);
    Timer timer;

    if (!renderer.Initialize())
        return -1;

    renderer.SetRenderMode(RenderMode::Wireframe);
    renderer.GetCamera().position.z = -200;

    while (renderer.ProcessEvents())
    {
        timer.Update();
        renderer.GetStats().fps = timer.GetFPS();
        renderer.GetStats().frameTime = timer.GetFrameTime();
        Update(timer);
        Render(renderer);
    }

    return 0;
}