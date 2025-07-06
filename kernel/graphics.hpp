#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <cstdint>
#include <cstddef>

#include "frame_buffer_config.hpp"

struct PixelColor
{
    uint8_t r, g, b;
};

class PixelWriter
{
public:
    explicit PixelWriter(const FrameBufferConfig& config) : config(config)
    {
    }

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

    void write(int x, int y, const PixelColor& color) override;

    void* operator new(size_t size, void* buf)
    {
        return buf;
    }

    void operator delete(void* obj) noexcept
    {
    }
};


class BGRResv8BitPerColorPixelWriter final : public PixelWriter
{
public:
    using PixelWriter::PixelWriter;

    void write(int x, int y, const PixelColor& color) override;

    void* operator new(size_t size, void* buf)
    {
        return buf;
    }

    void operator delete(void* obj) noexcept
    {
    }
};

template <typename T>
struct Vector2D
{
    T x, y;

    template <typename U>
    Vector2D& operator +=(const Vector2D<U>& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
};

void fill_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& color);
void draw_rectangle(PixelWriter& writer, const Vector2D<int>& pos, const Vector2D<int>& size, const PixelColor& color);


#endif //GRAPHICS_HPP
