#pragma once

#include <cstdint>
#include <span>

using uint8 = uint8_t;
using int8 = int8_t;
using uint16 = uint16_t;
using int16 = int16_t;
using uint32 = uint32_t;
using int32 = int32_t;

using usize = uint16;
using uptr = uint16;

template<typename T>
concept Address = sizeof(T) == sizeof(uptr);

inline constexpr uint8* to_ptr(uptr addr) {
    return reinterpret_cast<uint8*>(addr);
}

using mem = volatile uint8*;
using mem_map = std::span<mem>;