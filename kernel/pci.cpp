#include "pci.hpp"
#include "asmfunc.hpp"

namespace
{
  using namespace pci;

  uint32_t make_address(const uint8_t bus, const uint8_t device, const uint8_t function, const uint8_t reg_addr)
  {
    auto shl = [](const uint32_t x, const unsigned int bits)
    {
      return x << bits;
    };

    return shl(1, 31)
      | shl(bus, 16)
      | shl(device, 11)
      | shl(function, 8)
      | reg_addr & 0xfcu;
  }

  Error add_device(const Device& device)
  {
    if (num_devices == devices.size())
    {
      return MAKE_ERROR(Error::kFull);
    }

    devices[num_devices++] = device;
    ++num_devices;
    return MAKE_ERROR(Error::kSuccess);
  }

  Error scan_bus(uint8_t bus);

  /** @brief 指定のファンクションを devices に追加する．
   * もし PCI-PCI ブリッジなら，セカンダリバスに対し ScanBus を実行する
   */
  Error scan_function(uint8_t bus, uint8_t device, uint8_t function)
  {
    auto class_code = read_class_code(bus, device, function);
    auto header_type = read_header_type(bus, device, function);
    Device dev{bus, device, function, header_type, class_code};
    if (auto err = add_device(dev))
    {
      return err;
    }

    if (class_code.match(0x06u, 0x04u))
    {
      // standard PCI-PCI bridge
      auto bus_numbers = read_bus_numbers(bus, device, function);
      uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
      return scan_bus(secondary_bus);
    }

    return MAKE_ERROR(Error::kSuccess);
  }

  /** @brief 指定のデバイス番号の各ファンクションをスキャンする．
   * 有効なファンクションを見つけたら ScanFunction を実行する．
   */
  Error scan_device(uint8_t bus, uint8_t device)
  {
    if (auto err = scan_function(bus, device, 0))
    {
      return err;
    }
    if (is_single_function_device(read_header_type(bus, device, 0)))
    {
      return MAKE_ERROR(Error::kSuccess);
    }

    for (uint8_t function = 1; function < 8; ++function)
    {
      if (read_vendor_id(bus, device, function) == 0xffffu)
      {
        continue;
      }
      if (auto err = scan_function(bus, device, function))
      {
        return err;
      }
    }
    return MAKE_ERROR(Error::kSuccess);
  }

  /** @brief 指定のバス番号の各デバイスをスキャンする．
   * 有効なデバイスを見つけたら ScanDevice を実行する．
   */
  Error scan_bus(uint8_t bus)
  {
    for (uint8_t device = 0; device < 32; ++device)
    {
      if (read_vendor_id(bus, device, 0) == 0xffffu)
      {
        continue;
      }
      if (auto err = scan_device(bus, device))
      {
        return err;
      }
    }
    return MAKE_ERROR(Error::kSuccess);
  }
}

namespace pci
{
  void write_address(const uint32_t address)
  {
    IoOut32(CONFIG_ADDRESS, address);
  }

  void write_data(const uint32_t value)
  {
    IoOut32(CONFIG_DATA, value);
  }

  uint32_t read_data()
  {
    return IoIn32(CONFIG_DATA);
  }

  uint16_t read_vendor_id(const uint8_t bus, const uint8_t device, const uint8_t function)
  {
    write_address(make_address(bus, device, function, 0x00));
    return read_data() & 0xffffu;
  }

  uint16_t read_device_id(uint8_t bus, uint8_t device, uint8_t function)
  {
    write_address(make_address(bus, device, function, 0x00));
    return read_data() >> 16;
  }

  uint8_t read_header_type(uint8_t bus, uint8_t device, uint8_t function)
  {
    write_address(make_address(bus, device, function, 0x0c));
    return (read_data() >> 16) & 0xffu;
  }

  ClassCode read_class_code(uint8_t bus, uint8_t device, uint8_t function)
  {
    write_address(make_address(bus, device, function, 0x08));
    auto reg = read_data();
    ClassCode cc;
    cc.base = (reg >> 24) & 0xffu;
    cc.sub = (reg >> 16) & 0xffu;
    cc.interface = (reg >> 8) & 0xffu;
    return cc;
  }

  uint32_t read_bus_numbers(uint8_t bus, uint8_t device, uint8_t function)
  {
    write_address(make_address(bus, device, function, 0x18));
    return read_data();
  }

  bool is_single_function_device(uint8_t header_type)
  {
    return (header_type & 0x80u) == 0;
  }

  Error scan_all_bus()
  {
    num_devices = 0;

    auto header_type = read_header_type(0, 0, 0);
    if (is_single_function_device(header_type))
    {
      return scan_bus(0);
    }

    for (uint8_t function = 0; function < 8; ++function)
    {
      if (read_vendor_id(0, 0, function) == 0xffffu)
      {
        continue;
      }
      if (auto err = scan_bus(function))
      {
        return err;
      }
    }
    return MAKE_ERROR(Error::kSuccess);
  }

  uint32_t read_conf_reg(const Device& dev, uint8_t reg_addr)
  {
    write_address(make_address(dev.bus, dev.device, dev.function, reg_addr));
    return read_data();
  }

  void write_conf_reg(const Device& dev, uint8_t reg_addr, uint32_t value)
  {
    write_address(make_address(dev.bus, dev.device, dev.function, reg_addr));
    write_data(value);
  }

  WithError<uint64_t> read_bar(Device& device, unsigned int bar_index)
  {
    if (bar_index >= 6)
    {
      return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
    }

    const auto addr = calc_bar_address(bar_index);
    const auto bar = read_conf_reg(device, addr);

    // 32 bit address
    if ((bar & 4u) == 0)
    {
      return {bar, MAKE_ERROR(Error::kSuccess)};
    }

    // 64 bit address
    if (bar_index >= 5)
    {
      return {0, MAKE_ERROR(Error::kIndexOutOfRange)};
    }

    const auto bar_upper = read_conf_reg(device, addr + 4);
    return {
      bar | (static_cast<uint64_t>(bar_upper) << 32),
      MAKE_ERROR(Error::kSuccess)
    };
  }
}




