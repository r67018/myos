#ifndef PCI_HPP
#define PCI_HPP

#include <array>
#include <cstdint>

#include "error.hpp"

namespace pci
{
    constexpr uint16_t CONFIG_ADDRESS = 0x0cf8;
    constexpr uint16_t CONFIG_DATA = 0x0cfc;

    struct ClassCode
    {
        uint8_t base, sub, interface;

        [[nodiscard]] bool match(const uint8_t b) const
        {
            return b == base;
        }

        [[nodiscard]] bool match(const uint8_t b, const uint8_t s) const
        {
            return match(b) && s == sub;
        }

        [[nodiscard]] bool match(const uint8_t b, const uint8_t s, const uint8_t i) const
        {
            return match(b, s) && i == interface;
        }
    };

    struct Device
    {
        uint8_t bus, device, function, header_type;
        ClassCode class_code;
    };

    void write_address(uint32_t address);
    void write_data(uint32_t value);
    uint32_t read_data();

    uint16_t read_vendor_id(uint8_t bus, uint8_t device, uint8_t function);
    uint16_t read_device_id(uint8_t bus, uint8_t device, uint8_t function);
    uint8_t read_header_type(uint8_t bus, uint8_t device, uint8_t function);
    ClassCode read_class_code(uint8_t bus, uint8_t device, uint8_t function);

    inline uint16_t read_vendor_id(const Device& device)
    {
        return read_vendor_id(device.bus, device.device, device.function);
    }

    uint32_t read_conf_reg(const Device& dev, uint8_t reg_addr);
    void write_conf_reg(const Device& dev, uint8_t reg_addr, uint32_t value);

    uint32_t read_bus_numbers(uint8_t bus, uint8_t device, uint8_t function);
    bool is_single_function_device(uint8_t header_type);

    // 発見されたPCIデバイス
    inline std::array<Device, 32> devices;
    inline int num_devices;
    // PCIデバイスを探索
    Error scan_all_bus();

    constexpr uint8_t calc_bar_address(unsigned int bar_index)
    {
        return 0x10 + bar_index * 4;
    }

    WithError<uint64_t> read_bar(Device& device, unsigned int bar_index);

    // PCIケーパビリティレジスタの共通ヘッダ
    union CapabilityHeader
    {
        uint32_t data;

        struct
        {
            uint32_t cap_id : 8;
            uint32_t next_ptr : 8;
            uint32_t cap : 16;
        } __attribute__((packed)) bits;
    } __attribute__((packed));

    constexpr uint8_t CAPABILITY_MSI = 0x05;
    constexpr uint8_t CAPABILITY_MSIX = 0x11;

    // 指定されたPCIデバイスの指定されたケーパビリティレジスタを詠み込む
    CapabilityHeader read_capability_header(const Device& dev, uint8_t addr);

    // MSIケーパビリティ構造
    struct MSICapability
    {
        union
        {
            uint32_t data;

            struct
            {
                uint32_t cap_id : 8;
                uint32_t next_ptr : 8;
                uint32_t msi_enable : 1;
                uint32_t multi_msg_capable : 3;
                uint32_t multi_msg_enable : 3;
                uint32_t addr_64_capable : 1;
                uint32_t per_vector_mask_capable : 1;
                uint32_t  : 7;
            } __attribute__((packed)) bits;
        } __attribute__((packed)) header;

        uint32_t msg_addr;
        uint32_t msg_upper_addr;
        uint32_t msg_data;
        uint32_t mask_bits;
        uint32_t pending_bits;
    } __attribute__((packed));

    // MSIまたはMSI-X割り込みを設定
    Error configure_msi(const Device& dev, uint32_t msg_addr, uint32_t msg_data, unsigned int num_vector_exponent);

    enum class MSITriggerMode
    {
        Edge = 0,
        Level = 1,
    };

    enum class MSIDeliveryMode
    {
        Fixed = 0b000,
        LowestPriority = 0b001,
        SMI = 0b010,
        NMI = 0b100,
        INIT = 0b101,
        ExtINT = 0b11,
    };

    Error configure_msi_fixed_destination(const Device& dev,
                                          uint8_t apic_id,
                                          MSITriggerMode trigger_mode,
                                          MSIDeliveryMode delivery_mode,
                                          uint8_t vector,
                                          unsigned int num_vector_exponent);
}

#endif //PCI_HPP
