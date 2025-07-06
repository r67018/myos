#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

struct PixelColor {
    uint8_t r, g, b;
};

class PixelWriter
{
public:
    explicit PixelWriter(const FrameBufferConfig& config) : config(config) {}
    virtual ~PixelWriter() = default;
    virtual void write(int x, int y, const PixelColor& color) = 0;

protected:
    [[nodiscard]] uint8_t* pixel_at(const int x, const int y) const
    {
        return config.frame_buffer + 4 * (config.pixels_per_scan_line * y + x);
    }

private:
    const FrameBufferConfig& config;
};

class RGBResv8BitPerColorPixelWriter final : public PixelWriter
{
public:
    using PixelWriter::PixelWriter;

    void write(int x, int y, const PixelColor& color) override
    {
        auto p = pixel_at(x, y);
        p[0] = color.r;
        p[1] = color.g;
        p[2] = color.b;
    }

    void* operator new(size_t size, void* buf) {
        return buf;
    }

    void operator delete(void* obj) noexcept {}
};

class BGRResv8BitPerColorPixelWriter final : public PixelWriter
{
public:
    using PixelWriter::PixelWriter;

    void write(int x, int y, const PixelColor& color) override
    {
        auto p = pixel_at(x, y);
        p[0] = color.b;
        p[1] = color.g;
        p[2] = color.r;
    }

    void* operator new(size_t size, void* buf) {
        return buf;
    }

    void operator delete(void* obj) noexcept {}
};


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
    while (true) __asm__("hlt");
}

