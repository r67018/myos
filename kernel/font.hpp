#ifndef FONT_HPP
#define FONT_HPP

#include <cstdint>

#include "graphics.hpp"


void write_ascii(PixelWriter& writer, int x, int y, char c, const PixelColor& color);
void write_string(PixelWriter& writer, int x, int y, const char* s, const PixelColor& color);


#endif //FONT_HPP
