#ifndef MEMORY_MAP_HPP
#define MEMORY_MAP_HPP

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif // __cplusplus

struct MemoryMap
{
    unsigned long long buffer_size;
    void* buffer;
    unsigned long long map_size;
    unsigned long long map_key;
    unsigned long long descriptor_size;
    uint32_t descriptor_version;
};

struct MemoryDescriptor
{
    uint32_t type;
    uintptr_t physical_start;
    uintptr_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
};

#ifdef __cplusplus
enum class MemoryType
{
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
};

inline bool operator==(const uint32_t lhs, MemoryType rhs)
{
    return lhs == static_cast<uint32_t>(rhs);
}

inline bool operator==(MemoryType lhs, const uint32_t rhs)
{
    return static_cast<uint32_t>(lhs) == rhs;
}

inline bool is_available(const MemoryType type)
{
    return
        type == MemoryType::EfiBootServicesCode ||
        type == MemoryType::EfiBootServicesData ||
        type == MemoryType::EfiConventionalMemory;
}

constexpr int UEFI_PAGE_SIZE = 4096;
#endif // __cplusplus

#endif //MEMORY_MAP_HPP
