#include <cstdint>
#include <cstdio>

#include "asmfunc.hpp"
#include "frame_buffer_config.hpp"
#include "console.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"
#include "logger.hpp"
#include "pci.hpp"
#include "mouse.hpp"
#include "usb/xhci/xhci.hpp"
#include "usb/classdriver/mouse.hpp"

constexpr PixelColor DESKTOP_BG_COLOR{45, 118, 237};
constexpr PixelColor DESKTOP_FG_COLOR{255, 255, 255};


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

char mouse_cursor_buf[sizeof(MouseCursor)];
MouseCursor* mouse_cursor;

void mouse_observer(int8_t displacement_x, int8_t displacement_y)
{
    mouse_cursor->move_relative({displacement_x, displacement_y});
}

usb::xhci::Controller* xhc;

// xHCの割り込みハンドラ
__attribute__((interrupt))
void int_handler_xhci(InterruptFrame* frame)
{
    while (xhc->PrimaryEventRing()->HasFront())
    {
        if (auto err = usb::xhci::ProcessEvent(*xhc))
        {
            log(kError, "Error while ProcessingEvent: %s at %s:%d\n",
                err.Name(), err.File(), err.Line());
        }
    }
    notify_end_of_interrupt();
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

    // マウスカーソルのインスタンス生成
    mouse_cursor = new(mouse_cursor_buf) MouseCursor{pixel_writer, DESKTOP_BG_COLOR, {300, 200}};

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

    // 割り込みベクタを設定してIDTをCPUに登録
    const uint16_t cs = GetCS();
    set_IDT_entry(idt[InterruptVector::XHCI], make_IDT_attr(DescriptorType::InterruptGate, 0),
                  reinterpret_cast<uint64_t>(int_handler_xhci), cs);
    LoadIDT(sizeof(idt) - 1, reinterpret_cast<uintptr_t>(&idt[0]));
    // MSI割り込みを有効化
    // Destination ID(CPUコア番号)
    // 0xfee00020番地の31:24ビットにプログラムが動作しているコアのLocal APCI IDを取得できる。
    // 他のコアはまだ停止しているため、BSP(BootStrap Processor)のLocal APIC IDが得られる。
    const uint8_t bsp_local_apic_id = *reinterpret_cast<const uint32_t*>(0xfee00020) >> 24;
    pci::configure_msi_fixed_destination(*xhc_dev, bsp_local_apic_id, pci::MSITriggerMode::Level,
                                         pci::MSIDeliveryMode::Fixed, InterruptVector::XHCI, 0);

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

    ::xhc = &xhc;
    __asm__("sti"); // 割り込みを有効化

    // マウスのオブザーバーを設定
    usb::HIDMouseDriver::default_observer = mouse_observer;

    // USBを調べて接続済みポートの設定を行う。
    for (int i = 1; i <= xhc.MaxPorts(); ++i)
    {
        auto port = xhc.PortAt(i);
        log(kDebug, "Port %d: IsConnected=%d\n", i, port.IsConnected());

        if (port.IsConnected())
        {
            if (auto err = usb::xhci::ConfigurePort(xhc, port))
            {
                log(kError, "Failed to configure port: %s at %s:%d\n",
                    err.Name(), err.File(), err.Line());
                continue;
            }
        }
    }

    while (true) __asm__("hlt");
}
