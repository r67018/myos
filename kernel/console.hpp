
#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "graphics.hpp"

class Console
{
public:
    static constexpr int ROWS = 25;
    static constexpr int COLUMNS = 80;

    Console(PixelWriter& writer, const PixelColor& fg_color, const PixelColor& bg_color)
        : writer(writer), fg_color(fg_color), bg_color(bg_color),
          buffer{}, cursor_row{0}, cursor_column{0}
    {
    }

    void put_string(const char* s);

    void* operator new(size_t size, void* buf)
    {
        return buf;
    }

    void operator delete(void* obj) noexcept
    {
    }

private:
    void newline();

    PixelWriter& writer;
    const PixelColor& fg_color;
    const PixelColor& bg_color;
    char buffer[ROWS][COLUMNS + 1];
    int cursor_row;
    int cursor_column;
};


#endif //CONSOLE_HPP
