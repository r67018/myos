#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

char console_buf[sizeof(Console)];
Console* console;

int printk(const char* format, ...)
{
    va_list ap;
    char s[1024];

    va_start(ap, format);
    const int result = vsnprintf(s, sizeof(s), format, ap);
    va_end(ap);

    console->put_string(s);
    return result;
}

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
    auto fg_color = PixelColor{0, 0, 0};
    auto bg_color = PixelColor{255, 255, 255};
    console = new(console_buf) Console{*pixel_writer, fg_color, bg_color};

    for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x)
    {
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y)
        {
            pixel_writer->write(x, y, {255, 255, 255});
        }
    }

    for (int i = 0; i < 27; ++i)
    {
        printk("printk: %d\n", i);
    }

    while (true) __asm__("hlt");
}



