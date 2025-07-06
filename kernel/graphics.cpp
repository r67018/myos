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

void fill_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& color)
{
    for (int dy = 0; dy < size.y; ++dy)
    {
        for (int dx = 0; dx < size.x; ++dx)
        {
            writer.write(pos.x + dx, pos.y + dy, color);
        }
    }
}

void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& color)
{
    for (int dx = 0; dx < size.x; ++dx)
    {
        writer.write(pos.x + dx, pos.y, color);
        writer.write(pos.x + dx, pos.y + size.y - 1, color);
    }
    for (int dy = 1; dy < size.y - 1; ++dy)
    {
        writer.write(pos.x, pos.y + dy, color);
        writer.write(pos.x + size.x - 1, pos.y + dy, color);
    }
}

