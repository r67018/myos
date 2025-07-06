#ifndef MOUSE_HPP
#define MOUSE_HPP
#include "graphics.hpp"


class MouseCursor
{
public:
    MouseCursor(PixelWriter* writer, PixelColor erase_color, Vector2D<int> initial_position);
    void move_relative(Vector2D<int> displacement);

private:
    PixelWriter* pixel_writer;
    PixelColor erase_color;
    Vector2D<int> position;
};


#endif //MOUSE_HPP
