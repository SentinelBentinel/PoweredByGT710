#include "Renderer.h"

#include <algorithm>
#include <iostream>
#include <cmath>

Renderer::Renderer(int w, int h) : width(w), height(h), framebufer(w * h) {};

Renderer::~Renderer() {
    if (texture)
        SDL_DestroyTexture(texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Renderer::Initialize() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init err: "<< SDL_GetError() << '\n';
        return false;
    }
    if (!SDL_CreateWindowAndRenderer("PR3TEST", width, height,0,&window,&renderer)) {
        std::cerr << "Window/Renderer err: " << SDL_GetError() << '\n';
        return false;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );

    if (!texture) {
        std::cerr << "Texture err: " << SDL_GetError() << '\n';
        return false;
    }

    return true;
}

bool Renderer::ProcessEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;
    }
    return true;
}

void Renderer::Clear(Color color) {
    std::fill(
        framebufer.begin(),
        framebufer.end(),
        color
    );
}

void Renderer::Present() {
    SDL_UpdateTexture(
        texture,
        nullptr,
        framebufer.data(),
        width * sizeof(Color)
    );

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer,texture,nullptr,nullptr);
    SDL_RenderPresent(renderer);
}

std::vector<Color>& Renderer::GetFramebuffer() {
    return framebufer;
}

void Renderer::SetRenderMode(RenderMode mode) {
    renderMode = mode;
}

void Renderer::DrawPixel(int x, int y, Color color) {
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;
    framebufer[y * width + x] = color;
}

void Renderer::DrawLine(Vector2 start, Vector2 end, Color color) {
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

    if (steps == 0)
    {
        DrawPixel(
            static_cast<int>(start.x),
            static_cast<int>(start.y),
            color
        );
        return;
    }

    float xIncrement = dx / steps;
    float yIncrement = dy / steps;

    float x = start.x;
    float y = start.y;

    for (int i = 0; i <= steps; i++)
    {
        DrawPixel(
            static_cast<int>(std::round(x)),
            static_cast<int>(std::round(y)),
            color
        );

        x += xIncrement;
        y += yIncrement;
    }
}

void Renderer::RasterizeTriangle(const Triangle& triangle) {
    Vertex v0 = TransformVertex(triangle.v0, triangle.transform);
    Vertex v1 = TransformVertex(triangle.v1, triangle.transform);
    Vertex v2 = TransformVertex(triangle.v2, triangle.transform);

    float area = EdgeFunc(
        v0.position,
        v1.position,
        v2.position);

    int minX = std::max(0, static_cast<int>(std::floor(std::min({v0.position.x, v1.position.x, v2.position.x}))));
    int maxX = std::min(width - 1, static_cast<int>(std::ceil(std::max({v0.position.x, v1.position.x, v2.position.x}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({v0.position.y, v1.position.y, v2.position.y}))));
    int maxY = std::min(height - 1, static_cast<int>(std::ceil(std::max({v0.position.y, v1.position.y, v2.position.y}))));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 p = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
            float w0 = EdgeFunc(v1.position, v2.position, p);
            float w1 = EdgeFunc(v2.position, v0.position, p);
            float w2 = EdgeFunc(v0.position, v1.position, p);

            if ((area >= 0 && w0 >= 0 && w1 >= 0 && w2 >= 0) || (area < 0 && w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                w0 /= area;
                w1 /= area;
                w2 /= area;

                Color color;

                color.r = static_cast<unsigned char>(w0 * v0.color.r + w1 * v1.color.r + w2 * v2.color.r);
                color.g = static_cast<unsigned char>(w0 * v0.color.g + w1 * v1.color.g + w2 * v2.color.g);
                color.b = static_cast<unsigned char>(w0 * v0.color.b + w1 * v1.color.b + w2 * v2.color.b);

                framebufer[y * width + x] = color;
            }
        }
    }
}

void Renderer::RenderMesh(const Mesh& mesh) {
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        Triangle triangle;

        triangle.v0 = mesh.vertices[mesh.indices[i]];
        triangle.v1 = mesh.vertices[mesh.indices[i + 1]];
        triangle.v2 = mesh.vertices[mesh.indices[i + 2]];

        triangle.transform = mesh.transform;

        if (renderMode == RenderMode::Filled) {
            RasterizeTriangle(triangle);
        } else {
            Vertex v0 = TransformVertex(triangle.v0, triangle.transform);
            Vertex v1 = TransformVertex(triangle.v1, triangle.transform);
            Vertex v2 = TransformVertex(triangle.v2, triangle.transform);

            DrawLine(v0.position, v1.position, v0.color);
            DrawLine(v1.position, v2.position, v1.color);
            DrawLine(v2.position, v0.position, v2.color);
        }
    }
}

float Renderer::EdgeFunc(const Vector2& v1, const Vector2& v2, const Vector2& p) {
    return (p.x - v1.x) * (v2.y - v1.y) - (p.y - v1.y) * (v2.x - v1.x);
}

Vertex Renderer::TransformVertex(const Vertex& vertex, const Transform& transform) {
    Vertex result = vertex;

    // Scale
    result.position.x *= transform.scale;
    result.position.y *= transform.scale;

    // Rotate
    float c = std::cos(transform.rotation);
    float s = std::sin(transform.rotation);

    float x = result.position.x;
    float y = result.position.y;

    result.position.x = x * c - y * s;
    result.position.y = x * s + y * c;

    // Translate
    result.position.x += transform.position.x;
    result.position.y += transform.position.y;

    return result;
}