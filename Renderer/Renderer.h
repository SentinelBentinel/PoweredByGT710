#pragma once

#include "SDL3/SDL.h"
#include <vector>

#include "../Math/Color.h"
#include "../Graphics/Triangle.h"
#include "../Graphics/Mesh.h"
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"
#include "../Math/Matrix4.h"

enum class RenderMode
{
    Filled,
    Wireframe
};

class Renderer
{
public:
    Renderer(int width, int height);
    ~Renderer();

    float fov = 90.0f;

    bool Initialize();

    bool ProcessEvents();

    void Clear(Color color);

    void DrawPixel(int x, int y, Color color);
    void DrawLine(Vector2 start, Vector2 end, Color color);
    void RasterizeTriangle(const Triangle &triangle);
    void RenderMesh(const Mesh &mesh);
    void SetRenderMode(RenderMode mode);

    void Present();

    std::vector<Color> &GetFramebuffer();

private:
    float EdgeFunc(const Vector2 &v1, const Vector2 &v2, const Vector2 &p);
    Vertex TransformVertex(const Vertex &vertex, const Transform &transform);
    Vector2 ProjectVertex(const Vector3 &position);
    bool IsBackFace(const Vertex &v0, const Vertex &v1, const Vertex &v2);
    RenderMode renderMode = RenderMode::Filled;

    int width;
    int height;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;

    std::vector<Color> framebufer;
};