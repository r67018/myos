#ifndef INTERRUPT_HPP
#define INTERRUPT_HPP

#include <array>

#include "x86_descriptor.hpp"

union InterruptDescriptorAttribute {
    uint16_t data;

    struct {
        uint16_t interrupt_stack_table: 3;
        uint16_t : 5;
        DescriptorType type: 4;
        uint16_t : 1;
        uint16_t descriptor_privilege_level: 2;
        uint16_t present: 1;
    } __attribute__((packed)) bits;
} __attribute__((packed));

struct InterruptDescriptor {
    uint16_t offset_low;
    uint16_t segment_selector;
    InterruptDescriptorAttribute attr;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

inline std::array<InterruptDescriptor, 256> idt;

constexpr InterruptDescriptorAttribute make_IDT_attr(
    const DescriptorType type,
    const uint8_t descriptor_privilege_level,
    const bool present = true,
    const uint8_t interrupt_stack_table = 0) {
    InterruptDescriptorAttribute attr{};
    attr.bits.interrupt_stack_table = interrupt_stack_table;
    attr.bits.type = type;
    attr.bits.descriptor_privilege_level = descriptor_privilege_level;
    attr.bits.present = present;
    return attr;
}

void set_IDT_entry(InterruptDescriptor &desc,
                   InterruptDescriptorAttribute attr,
                   uint64_t offset,
                   uint16_t segment_selector);

class InterruptVector {
public:
    enum Number {
        XHCI = 0x40,
    };
};

struct InterruptFrame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void notify_end_of_interrupt();

#endif //INTERRUPT_HPP
