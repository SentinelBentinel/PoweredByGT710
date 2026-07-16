#include "Texture.h"

#include <algorithm>
#include <iostream>

bool Texture::Load(const std::string &path)
{
    SDL_Surface *surface = IMG_Load(path.c_str());

    if (!surface)
    {
        std::cerr << "Failed to load texture: " << path << '\n';
        return false;
    }

    SDL_Surface *converted = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGB24);

    SDL_DestroySurface(surface);

    if (!converted)
    {
        std::cerr << "Failed to convert texture.\n";
        return false;
    }

    width = converted->w;
    height = converted->h;

    pixels.resize(width * height);

    uint8_t *src = static_cast<uint8_t*>(converted->pixels);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int i = y * width + x;

            pixels[i].r = src[i * 3 + 0];
            pixels[i].g = src[i * 3 + 1];
            pixels[i].b = src[i * 3 + 2];
        }
    }

    SDL_DestroySurface(converted);

    std::cout << "Loaded texture "
              << path
              << " ("
              << width
              << "x"
              << height
              << ")\n";

    return true;
}

Color Texture::Sample(const Vector2 &uv) const
{
    if (pixels.empty())
        return {255, 0, 255};

    float u = uv.x - std::floor(uv.x);
    float v = uv.y - std::floor(uv.y);

    int x = std::clamp(static_cast<int>(u * (width - 1)), 0, width - 1);
    int y = std::clamp(static_cast<int>((1.0f - v) * (height - 1)), 0, height - 1);

    return pixels[y * width + x];
}