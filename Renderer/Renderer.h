#pragma once

#include "SDL3/SDL.h"
#include <vector>

#include "../Math/Color.h"
#include "../Graphics/Triangle.h"
#include "../Graphics/Mesh.h"

enum class RenderMode {
    Filled,
    Wireframe
};

class Renderer {
public: 
    Renderer(int width, int height);
    ~Renderer();

    bool Initialize();

    bool ProcessEvents();

    void Clear(Color color);

    void DrawPixel(int x, int y, Color color);
    void DrawLine(Vector2 start, Vector2 end, Color color);
    void RasterizeTriangle(const Triangle& triangle);
    void RenderMesh(const Mesh& mesh);
    void SetRenderMode(RenderMode mode);

    void Present();

    std::vector<Color>& GetFramebuffer();
private:
    float EdgeFunc(const Vector2& v1, const Vector2& v2, const Vector2& p);
    Vertex TransformVertex(const Vertex& vertex, const Transform& transform);
    RenderMode renderMode = RenderMode::Filled;

    int width;
    int height;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    std::vector<Color> framebufer;
};