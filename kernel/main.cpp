#include <cstdint>

#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

extern "C" [[noreturn]] void KernelMain(const FrameBufferConfig& frame_buffer_config) {
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
        for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
            pixel_writer->write(x, y, {255, 255, 255});
        }
    }
    for (int x = 0; x < 200; ++x)
    {
        for (int y = 0; y < 100; ++y)
        {
            pixel_writer->write(x, y, {0, 255, 0});
        }
    }

    for (int i = 0; i < 127; ++i)
    {
        char c = 'A' + i;
        int x = 50 + i * 8;
        write_ascii(*pixel_writer, x, 50, c, {0, 0, 0});
    }
    while (true) __asm__("hlt");
}

