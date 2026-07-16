#pragma once

#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "../Math/Color.h"
#include "../Math/Vector2.h"

class Texture
{
public:
    bool Load(const std::string &path);

    Color Sample(const Vector2 &uv) const;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
private:
    int width = 0;
    int height = 0;

    std::vector<Color> pixels;
};