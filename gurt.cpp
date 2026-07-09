#include "Renderer/Renderer.h"
#include "Graphics/Triangle.h"
#include "Graphics/Mesh.h"

#include <vector>
#include <iostream>

Mesh square =
    {
        // Vertices
        {
            {{-100, -100}, {255, 0, 0}},
            {{100, -100}, {0, 255, 0}},
            {{100, 100}, {0, 0, 255}},
            {{-100, 100}, {255, 255, 0}}},

        // Indices
        {
            0, 1, 2,
            0, 2, 3},

        // Transform
        {
            {400, 300},
            0.0f,
            1.0f}};

std::vector<Mesh> meshes =
    {
        square
    };

void Update()
{
    for (Mesh &mesh : meshes)
    {
        mesh.transform.rotation += 0;
    }
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

    if (!renderer.Initialize())
        return -1;

    renderer.SetRenderMode(RenderMode::Filled);

    while (renderer.ProcessEvents())
    {
        Update();
        Render(renderer);
    }

    return 0;
}