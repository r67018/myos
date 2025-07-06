#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "pci.hpp"

constexpr PixelColor DESKTOP_BG_COLOR{45, 118, 237};
constexpr PixelColor DESKTOP_FG_COLOR{255, 255, 255};

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

    const int FRAME_WIDTH = static_cast<int>(frame_buffer_config.horizontal_resolution);
    const int FRAME_HEIGHT = static_cast<int>(frame_buffer_config.vertical_resolution);
    fill_rectangle(*pixel_writer, {0, 0}, {FRAME_WIDTH, FRAME_HEIGHT - 50}, DESKTOP_BG_COLOR);
    fill_rectangle(*pixel_writer, {0, FRAME_WIDTH - 50}, {FRAME_WIDTH, 50}, {1, 8, 17});
    fill_rectangle(*pixel_writer, {0, FRAME_WIDTH - 50}, {FRAME_WIDTH / 5, 50}, {80, 80, 80});
    draw_rectangle(*pixel_writer, {10, FRAME_HEIGHT - 40}, {30, 30}, {160, 160, 160});

    console = new(console_buf) Console{*pixel_writer, DESKTOP_FG_COLOR, DESKTOP_BG_COLOR};
    printk("Welcom to MikanOS!\n");

    auto err = pci::scan_all_bus();
    printk("scan_all_bus: %s\n", err.Name());

    for (int i = 0; i < pci::num_devices; ++i)
    {
        const auto& dev = pci::devices[i];
        auto vendor_id = pci::read_vendor_id(dev);
        auto class_code = pci::read_class_code(dev.bus, dev.device, dev.function);
        printk("%d.%d.%d: vend %04x, class %08x, head %02x\n",
               dev.bus, dev.device, dev.function,
               vendor_id, class_code, dev.header_type);
    }

    while (true) __asm__("hlt");
}



