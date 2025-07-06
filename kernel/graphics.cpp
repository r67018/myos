#include "graphics.hpp"

void RGBResv8BitPerColorPixelWriter::write(const int x, const int y, const PixelColor& color)
{
    const auto p = pixel_at(x, y);
    p[0] = color.r;
    p[1] = color.g;
    p[2] = color.b;
}

void BGRResv8BitPerColorPixelWriter::write(const int x, const int y, const PixelColor& color)
{
    const auto p = pixel_at(x, y);
    p[0] = color.b;
    p[1] = color.g;
    p[2] = color.r;
}
