#pragma once

#include <cstddef>
#include <cstdint>

namespace toybox {

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using f32 = float;
using byte = unsigned char;
using usize = std::size_t;

static_assert(sizeof(f32) == 4, "Toybox requires a 32-bit float.");

} // namespace toybox
