#include "console.hpp"

#include <cstring>

#include "font.hpp"

void Console::put_string(const char* s)
{
    while (*s)
    {
        if (*s == '\n')
        {
            newline();
        }
        else if (cursor_column < COLUMNS - 1)
        {
            write_ascii(writer, 8 * cursor_column, 16 * cursor_row, *s, fg_color);
            buffer[cursor_row][cursor_column] = *s;
            ++cursor_column;
        }
        ++s;
    }
}

void Console::newline()
{
    cursor_column = 0;
    if (cursor_row < ROWS - 1)
    {
        ++cursor_row;
    }
    else
    {
        for (int y = 0; y < 16 * ROWS; ++y)
        {
            for (int x = 0; x < 8 * COLUMNS; ++x)
            {
                writer.write(x, y, bg_color);
            }
        }
        for (int row = 0; row < ROWS - 1; ++row)
        {
            memcpy(buffer[row], buffer[row + 1], COLUMNS + 1);
            write_string(writer, 0, 16 * row, buffer[row], fg_color);
        }
        memset(buffer[ROWS - 1], 0, COLUMNS + 1);
    }
}


