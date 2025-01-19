#pragma once

#include "types.h"
// #include "span.h"

template<typename T>
concept Address = sizeof(T) == sizeof(uptr);

using mem = volatile uint8;
// using mem_map = std::span<mem>;

inline constexpr uint8* to_ptr(uptr addr) {
    return reinterpret_cast<uint8*>(addr);
}
inline constexpr uptr to_addr(mem* ptr) {
    return reinterpret_cast<uptr>(ptr);
}