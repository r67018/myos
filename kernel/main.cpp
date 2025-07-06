#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "font.hpp"
#include "logger.hpp"
#include "pci.hpp"
#include "usb/xhci/xhci.hpp"

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

void switch_ehci2xhci(pci::Device xhc_dev)
{
    bool intel_ehc_exist = false;
    for (int i = 0; i < pci::num_devices; ++i)
    {
        if (pci::devices[i].class_code.match(0x0cu, 0x03u, 0x20u) /* EHCI */ &&
            0x8086 == pci::read_vendor_id(pci::devices[i]))
        {
            intel_ehc_exist = true;
            break;
        }
    }
    if (!intel_ehc_exist)
    {
        return;
    }

    uint32_t superspeed_ports = pci::read_conf_reg(xhc_dev, 0xdc); // USB3PRM
    pci::write_conf_reg(xhc_dev, 0xd8, superspeed_ports); // USB3_PSSEN
    uint32_t ehci2xhci_ports = pci::read_conf_reg(xhc_dev, 0xd4); // XUSB2PRM
    pci::write_conf_reg(xhc_dev, 0xd0, ehci2xhci_ports); // XUSB2PR
    Log(kDebug, "SwitchEhci2Xhci: SS = %02, xHCI = %02x\n",
        superspeed_ports, ehci2xhci_ports);
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

    pci::Device* xhc_dev = nullptr;
    for (int i = 0; i < pci::num_devices; ++i)
    {
        if (pci::devices[i].class_code.match(0x0cu, 0x03u, 0x30u))
        {
            xhc_dev = &pci::devices[i];

            if (0x8086 == pci::read_vendor_id(*xhc_dev))
            {
                break;
            }
        }
    }

    if (xhc_dev)
    {
        log(kInfo, "xHC has been found: %d.%d.%d\n",
            xhc_dev->bus, xhc_dev->device, xhc_dev->function);
    }
    else
    {
        log(kError, "xHC has not been found\n");
    }

    const WithError<u_int64_t> xhc_bar = pci::read_bar(*xhc_dev, 0);
    log(kDebug, "ReadBar: %s\n", xhc_bar.error.Name());
    const uint64_t xhc_mmio_base = xhc_bar.value & ~static_cast<uint64_t>(0xf);
    log(kDebug, "xHC mmio_base: %08lx\n", xhc_mmio_base);

    usb::xhci::Controller xhc{xhc_mmio_base};

    if (0x8086 == pci::read_vendor_id(*xhc_dev))
    {
        switch_ehci2xhci(*xhc_dev);
    }
    {
        auto err = xhc.Initialize();
        log(kDebug, "xhc.Initialize: %s\n", err.Name());
    }

    log(kInfo, "xHC starting\n");
    xhc.Run();

    while (true) __asm__("hlt");
}



