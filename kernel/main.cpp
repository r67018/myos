#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

extern "C" [[noreturn]] void KernelMain(const FrameBufferConfig& frame_buffer_config)
{
    switch (frame_buffer_config.pixel_format)
    {
    case kPixelRGBResv8BitPerColor:
        pixel_writer = new(pixel_writer_buf) RGBResv8BitPerColorPixelWriter{frame_buffer_config};
        break;
    case kPixelBGRResv8BitPerColor:
        pixel_writer = new(pixel_writer_buf) BGRResv8BitPerColorPixelWriter{frame_buffer_config};
        break;
    }

    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x)
    {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y)
        {
            pixel_writer->write(x, y, {255, 255, 255});
        }
    }

    auto fg_color = PixelColor{0, 0, 0};
    auto bg_color = PixelColor{255, 255, 255};
    auto console = Console{*pixel_writer, fg_color, bg_color};
    for (int i = 0; i < 26; ++i)
    {
        char buffer[128];
        sprintf(buffer, "Line %d\n", i + 1);
        console.put_string(buffer);
    }

    while (true) __asm__("hlt");
}

