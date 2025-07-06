#include "mouse.hpp"

namespace
{
    constexpr int MOUSE_CURSOR_WIDTH = 15;
    constexpr int MOUSE_CURSOR_HEIGHT = 24;
    constexpr char MOUSE_CURSOR_SHAPE[MOUSE_CURSOR_HEIGHT][MOUSE_CURSOR_WIDTH + 1] = {
        "@              ",
        "@@             ",
        "@.@            ",
        "@..@           ",
        "@...@          ",
        "@....@         ",
        "@.....@        ",
        "@......@       ",
        "@.......@      ",
        "@........@     ",
        "@.........@    ",
        "@..........@   ",
        "@...........@  ",
        "@............@ ",
        "@......@@@@@@@@",
        "@......@       ",
        "@....@@.@      ",
        "@...@ @.@      ",
        "@..@   @.@     ",
        "@.@    @.@     ",
        "@@      @.@    ",
        "@       @.@    ",
        "         @.@   ",
        "         @@@   ",
    };

    void draw_mouse_cursor(PixelWriter* pixel_writer, const Vector2D<int> position)
    {
        for (int dy = 0; dy < MOUSE_CURSOR_HEIGHT; ++dy)
        {
            for (int dx = 0; dx < MOUSE_CURSOR_WIDTH; ++dx)
            {
                if (MOUSE_CURSOR_SHAPE[dy][dx] == '@')
                {
                    pixel_writer->write(position.x + dx, position.y + dy, {0, 0, 0});
                }
                else if (MOUSE_CURSOR_SHAPE[dy][dx] == '.')
                {
                    pixel_writer->write(position.x + dx, position.y + dy, {255, 255, 255});
                }
            }
        }
    }

    void erase_mouse_cursor(PixelWriter* pixel_writer, const Vector2D<int> position, const PixelColor erase_color)
    {
        for (int dy = 0; dy < MOUSE_CURSOR_HEIGHT; ++dy)
        {
            for (int dx = 0; dx < MOUSE_CURSOR_WIDTH; ++dx)
            {
                if (MOUSE_CURSOR_SHAPE[dy][dx] != ' ')
                {
                    pixel_writer->write(position.x + dx, position.y + dy, erase_color);
                }
            }
        }
    }
}

MouseCursor::MouseCursor(PixelWriter* writer, const PixelColor erase_color, const Vector2D<int> initial_position)
    : pixel_writer(writer),
      erase_color(erase_color),
      position(initial_position)
{
    draw_mouse_cursor(pixel_writer, position);
}

void MouseCursor::move_relative(const Vector2D<int> displacement)
{
    erase_mouse_cursor(pixel_writer, position, erase_color);
    position += displacement;
    draw_mouse_cursor(pixel_writer, position);
}
