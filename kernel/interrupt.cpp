#include "interrupt.hpp"

#include <cstdint>

void set_IDT_entry(InterruptDescriptor& desc,
                   const InterruptDescriptorAttribute attr,
                   const uint64_t offset,
                   const uint16_t segment_selector)
{
    desc.attr = attr;
    desc.offset_low = offset & 0xffffu;
    desc.offset_middle = (offset >> 16) & 0xffffu;
    desc.offset_high = offset >> 32;
    desc.segment_selector = segment_selector;
}

void notify_end_of_interrupt()
{
    const volatile auto end_of_interrupt = reinterpret_cast<uint32_t*>(0xfee000b0);
    *end_of_interrupt = 0;
}
