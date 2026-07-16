#include "Renderer.h"

#include <algorithm>
#include <iostream>
#include <format>
#include <limits>
#include <cmath>
#include <SDL3_ttf/SDL_ttf.h>
/*
    Poop
*/

float DistanceToPlane(const Plane &plane, const Vector3 &point)
{
    return Vector3::Dot(plane.normal, point) + plane.distance;
}

static Vertex IntersectPlane(const Vertex &inside, const Vertex &outside, const Plane &plane)
{
    float da = DistanceToPlane(plane, inside.position);
    float db = DistanceToPlane(plane, outside.position);
    float t = da / (da - db);

    return Vertex::Lerp(inside, outside, t);
}

Plane NormalizePlane(const Plane &plane)
{
    float length = plane.normal.Length();

    return {
        plane.normal / length,
        plane.distance / length};
}

Renderer::Renderer(int w, int h) : width(w), height(h), framebufer(w * h), depthBuffer(w * h) {};

Renderer::~Renderer()
{
    if (texture)
        SDL_DestroyTexture(texture);
    if (font)
        TTF_CloseFont(font);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();
}

Stats &Renderer::GetStats()
{
    return stats;
}

bool Renderer::Initialize()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init err: " << SDL_GetError() << '\n';
        return false;
    }
    if (!TTF_Init())
    {
        std::cerr << "TTF_Init err: " << SDL_GetError() << '\n';
    }
    if (!SDL_CreateWindowAndRenderer("PR3TEST", width, height, 0, &window, &renderer))
    {
        std::cerr << "Window/Renderer err: " << SDL_GetError() << '\n';
        return false;
    }

    font = TTF_OpenFont("../Assets/JetBrainsMono-Bold.ttf", 16);

    if (!font)
    {
        std::cerr << "Font err: " << SDL_GetError() << '\n';
        return false;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

    if (!texture)
    {
        std::cerr << "Texture err: " << SDL_GetError() << '\n';
        return false;
    }

    light.direction = light.direction.Normalized();

    SDL_SetWindowRelativeMouseMode(window, true);

    return true;
}

bool Renderer::ProcessEvents()
{
    SDL_Event event;
    const bool *keys = SDL_GetKeyboardState(nullptr);
    Camera &cam = GetCamera();

    float speed = 1.0f;
    float sensitivity = 0.003f;

    Vector3 forward = cam.Forward();
    Vector3 right = cam.Right();
    Vector3 up = Vector3::Cross(right, forward).Normalized();

    if (keys[SDL_SCANCODE_W])
        cam.position += forward * speed;
    if (keys[SDL_SCANCODE_S])
        cam.position -= forward * speed;
    if (keys[SDL_SCANCODE_A])
        cam.position -= right * speed;
    if (keys[SDL_SCANCODE_D])
        cam.position += right * speed;
    if (keys[SDL_SCANCODE_SPACE])
        cam.position -= up * speed;
    if (keys[SDL_SCANCODE_LCTRL])
        cam.position += up * speed;
    if (keys[SDL_SCANCODE_ESCAPE])
        return false;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return false;
        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            cam.rotation.y += event.motion.xrel * sensitivity;
            cam.rotation.x += event.motion.yrel * sensitivity;
        }
    }

    cam.rotation.x = std::clamp(cam.rotation.x, -1.55f, 1.55f);
    return true;
}

void Renderer::Clear(Color color)
{
    std::fill(
        framebufer.begin(),
        framebufer.end(),
        color);
    std::fill(
        depthBuffer.begin(),
        depthBuffer.end(),
        std::numeric_limits<float>::infinity());

    stats.vertices = 0;
    stats.triangles = 0;
    stats.trianglesCulled = 0;
    stats.meshes = 0;
    stats.pixelsDrawn = 0;
}

void Renderer::Present()
{
    SDL_UpdateTexture(
        texture,
        nullptr,
        framebufer.data(),
        width * sizeof(Color));

    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);

    Stats &s = GetStats();

    DrawText(10, 30,
             std::format("FPS: {:.0f}", s.fps),
             {255, 255, 255});

    DrawText(10, 50,
             std::format("Frame: {:.2f} ms", s.frameTime),
             {255, 255, 255});

    DrawText(10, 70,
             std::format("Meshes: {}", s.meshes),
             {255, 255, 255});

    DrawText(10, 90,
             std::format("Triangles: {}", s.triangles),
             {255, 255, 255});

    DrawText(10, 110,
             std::format("Vertices: {}", s.vertices),
             {255, 255, 255});

    DrawText(10, 130,
             std::format("Pixels: {}", s.pixelsDrawn),
             {255, 255, 255});

    DrawText(10, 150,
             std::format("Culled: {}", s.trianglesCulled),
             {255, 255, 255});

    SDL_RenderPresent(renderer);
}

std::vector<Color> &Renderer::GetFramebuffer()
{
    return framebufer;
}

void Renderer::SetRenderMode(RenderMode mode)
{
    renderMode = mode;
}

Vector3 Renderer::ComputeFaceNormal(const Vertex &v0, const Vertex &v1, const Vertex &v2)
{
    Vector3 edge1 = v1.position - v0.position;
    Vector3 edge2 = v2.position - v0.position;

    return Vector3::Cross(edge1, edge2).Normalized();
}

void Renderer::DrawPixel(int x, int y, Color color)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;
    framebufer[y * width + x] = color;
}

void Renderer::DrawLine(Vector2 start, Vector2 end, Color color)
{
    float dx = end.x - start.x;
    float dy = end.y - start.y;

    int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)));

    if (steps == 0)
    {
        DrawPixel(
            static_cast<int>(start.x),
            static_cast<int>(start.y),
            color);
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
            color);

        x += xIncrement;
        y += yIncrement;
    }
}

void Renderer::DrawText(int x, int y, const std::string &text, Color color)
{
    SDL_Color sdlColor{
        color.r,
        color.g,
        color.b,
        255};

    SDL_Surface *surface = TTF_RenderText_Blended(font, text.c_str(), 0, sdlColor);

    if (!surface)
        return;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FRect dst{
        (float)x,
        (float)y,
        (float)surface->w,
        (float)surface->h};

    SDL_RenderTexture(renderer, texture, nullptr, &dst);

    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
}

Camera &Renderer::GetCamera()
{
    return camera;
}

Vector2 Renderer::ProjectVertex(const Vector3 &position)
{
    static Matrix4 projection = Matrix4::Perspective(
        fov * (3.1415926535f / 180.0f),
        static_cast<float>(width) / height,
        0.1f,
        1000.0f);

    Vector4 clip = projection.MultiplyPoint4(position);

    if (clip.w <= 0.0f)
        return {-1000000.0f, -1000000.0f};

    float invW = 1.0f / clip.w;

    float ndcX = clip.x * invW;
    float ndcY = clip.y * invW;

    return {
        (ndcX + 1.0f) * 0.5f * width,
        (1.0f - ndcY) * 0.5f * height};
}

bool Renderer::IsBackFace(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2)
{
    float area = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);

    return area <= 0.0f;
}

void Renderer::RasterizeTriangleViewSpace(const Triangle &triangle)
{
    Vertex v0 = triangle.v0;
    Vertex v1 = triangle.v1;
    Vertex v2 = triangle.v2;

    Vector2 p0 = ProjectVertex(v0.position);
    Vector2 p1 = ProjectVertex(v1.position);
    Vector2 p2 = ProjectVertex(v2.position);

    float z0 = v0.position.z;
    float z1 = v1.position.z;
    float z2 = v2.position.z;

    float invZ0 = 1.0f / z0;
    float invZ1 = 1.0f / z1;
    float invZ2 = 1.0f / z2;

    float area = EdgeFunc(
        p0,
        p1,
        p2);

    if (std::abs(area) < 0.0001f)
        return;

    int minX = std::max(0, static_cast<int>(std::floor(std::min({p0.x, p1.x, p2.x}))));
    int maxX = std::min(width - 1, static_cast<int>(std::ceil(std::max({p0.x, p1.x, p2.x}))));
    int minY = std::max(0, static_cast<int>(std::floor(std::min({p0.y, p1.y, p2.y}))));
    int maxY = std::min(height - 1, static_cast<int>(std::ceil(std::max({p0.y, p1.y, p2.y}))));

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 p = {static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f};
            float w0 = EdgeFunc(p1, p2, p);
            float w1 = EdgeFunc(p2, p0, p);
            float w2 = EdgeFunc(p0, p1, p);

            if ((area >= 0 && w0 >= 0 && w1 >= 0 && w2 >= 0) || (area < 0 && w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                w0 /= area;
                w1 /= area;
                w2 /= area;

                float brightness = w0 * v0.brightness + w1 * v1.brightness + w2 * v2.brightness;

                float invZ = w0 * invZ0 + w1 * invZ1 + w2 * invZ2;

                float depth = 1.0f / invZ;

                Color color;

                float cW0 = (w0 * invZ0) / invZ;
                float cW1 = (w1 * invZ1) / invZ;
                float cW2 = (w2 * invZ2) / invZ;

                color.r = static_cast<unsigned char>((cW0 * v0.color.r + cW1 * v1.color.r + cW2 * v2.color.r) * brightness);
                color.g = static_cast<unsigned char>((cW0 * v0.color.g + cW1 * v1.color.g + cW2 * v2.color.g) * brightness);
                color.b = static_cast<unsigned char>((cW0 * v0.color.b + cW1 * v1.color.b + cW2 * v2.color.b) * brightness);

                int index = y * width + x;

                if (depth < depthBuffer[index])
                {
                    depthBuffer[index] = depth;

                    framebufer[index] = color;

                    stats.pixelsDrawn++;
                }
            }
        }
    }
}

void Renderer::RenderMesh(const Mesh &mesh)
{
    stats.meshes++;
    for (size_t i = 0; i < mesh.indices.size(); i += 3)
    {
        stats.triangles++;
        stats.vertices += 3;

        Triangle triangle;

        triangle.v0 = TransformVertex(mesh.vertices[mesh.indices[i]], mesh.transform);
        triangle.v1 = TransformVertex(mesh.vertices[mesh.indices[i + 1]], mesh.transform);
        triangle.v2 = TransformVertex(mesh.vertices[mesh.indices[i + 2]], mesh.transform);

        std::vector<Triangle> input;
        std::vector<Triangle> output;

        input.push_back(triangle);

        auto ClipStage = [&](const Plane &plane)
        {
            output.clear();

            for (const Triangle &t : input)
                PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(t, plane, output);

            input = output;

            if (input.empty())
                return;
        };

        std::vector<Triangle> clipped;

        Plane nearPlane = NormalizePlane({{0.0f, 0.0f, 1.0f},
                                          -0.1f});
        Plane farPlane = NormalizePlane({{0.0f, 0.0f, -1.0f},
                                         10000.0f});
        Plane leftPlane = NormalizePlane({{1.0f, 0.0f, 1.0f},
                                          0.0f});

        Plane rightPlane = NormalizePlane({{-1.0f, 0.0f, 1.0f},
                                           0.0f});

        Plane topPlane = NormalizePlane({{0.0f, -1.0f, 1.0f},
                                         0.0f});

        Plane bottomPlane = NormalizePlane({{0.0f, 1.0f, 1.0f},
                                            0.0f});
        ClipStage(nearPlane);
        if (input.empty()) continue;
        ClipStage(farPlane);
        if (input.empty()) continue;
        ClipStage(leftPlane);
        if (input.empty()) continue;
        ClipStage(rightPlane);
        if (input.empty()) continue;
        ClipStage(topPlane);
        if (input.empty()) continue;
        ClipStage(bottomPlane);
        if (input.empty()) continue;

        for (const Triangle &clippedTriangle : input)
        {
            Vertex v0 = clippedTriangle.v0;
            Vertex v1 = clippedTriangle.v1;
            Vertex v2 = clippedTriangle.v2;

            Vector2 p0 = ProjectVertex(v0.position);
            Vector2 p1 = ProjectVertex(v1.position);
            Vector2 p2 = ProjectVertex(v2.position);

            if (IsBackFace(p0, p1, p2))
            {
                stats.trianglesCulled++;
                continue;
            }

            if (renderMode == RenderMode::Filled)
            {
                RasterizeTriangleViewSpace(clippedTriangle);
            }
            else
            {
                Vector2 p0 = ProjectVertex(v0.position);
                Vector2 p1 = ProjectVertex(v1.position);
                Vector2 p2 = ProjectVertex(v2.position);

                DrawLine(p0, p1, v0.color);
                DrawLine(p1, p2, v1.color);
                DrawLine(p2, p0, v2.color);
            }
        }
    }
}

float Renderer::EdgeFunc(const Vector2 &v1, const Vector2 &v2, const Vector2 &p)
{
    return (p.x - v1.x) * (v2.y - v1.y) - (p.y - v1.y) * (v2.x - v1.x);
}

Vertex Renderer::TransformVertex(const Vertex &vertex, const Transform &transform)
{
    Vertex result = vertex;

    Matrix4 model =
        Matrix4::Translation(transform.position) *
        Matrix4::RotationZ(transform.rotation.z) *
        Matrix4::RotationY(transform.rotation.y) *
        Matrix4::RotationX(transform.rotation.x) *
        Matrix4::Scale(transform.scale);

    Vector3 world = model.MultiplyPoint(vertex.position);

    Vector3 view = camera.GetViewMatrix().MultiplyPoint(world);

    Vector3 worldNormal = model.MultiplyVector(vertex.normal).Normalized();

    Vector3 viewNormal = camera.GetViewMatrix().MultiplyVector(worldNormal).Normalized();

    result.position = view;
    result.normal = viewNormal;

    Vector3 lightDir =
    {
        -light.direction.x,
        -light.direction.y,
        -light.direction.z
    };

    result.brightness = std::max(light.ambient, Vector3::Dot(viewNormal, lightDir));

    return result;
}

void Renderer::PerformHomogenousCoordinateSpaceSutherlandHodgmanPolygonClippingAlgorithmOnInputTriangleAgainstTheNearZPlaneToPreventZeroDivison(const Triangle &triangle, const Plane &plane, std::vector<Triangle> &output)
{
    Vertex verts[3] =
        {
            triangle.v0,
            triangle.v1,
            triangle.v2};

    std::vector<Vertex> clippedPolygon;

    for (int i = 0; i < 3; i++)
    {
        const Vertex &current = verts[i];
        const Vertex &next = verts[(i + 1) % 3];

        float distCurrent = DistanceToPlane(plane, current.position);
        float distNext = DistanceToPlane(plane, next.position);

        if (distCurrent >= 0.0f)
        {
            clippedPolygon.push_back(current);

            if (distNext < 0.0f)
            {
                clippedPolygon.push_back(IntersectPlane(current,next,plane));
            }
        }

        else if (distNext >= 0.0f)
        {
            clippedPolygon.push_back(IntersectPlane(current, next, plane));
        }
    }

    if (clippedPolygon.size() < 3) return;

    for (size_t i = 1; i < clippedPolygon.size() - 1; i++)
    {
        Triangle t;
        t.v0 = clippedPolygon[0];
        t.v1 = clippedPolygon[i];
        t.v2 = clippedPolygon[i + 1];
        output.push_back(t);
    }
}