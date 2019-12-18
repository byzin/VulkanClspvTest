/*!
  \file config.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_CONFIG_HPP
#define CLSPV_TEST_CONFIG_HPP

// Standard C++ library
#include <cstdint>
#include <type_traits>

namespace clspvtest {

// General
// Integer types
using int8b = std::int8_t;
using int16b = std::int16_t;
using int32b = std::int32_t;
using int64b = std::int64_t;
// Unsigned integer types
using uint = unsigned int;
using uint8b = std::uint8_t;
using uint16b = std::uint16_t;
using uint32b = std::uint32_t;
using uint64b = std::uint64_t;

// Device

/*!
  */
enum class QueueType : uint32b
{
  kCompute = 0,
  kTransfer
};

// Buffer

/*!
  */
enum class BufferUsage : uint32b
{
  kDeviceOnly = 0b1u << 0,
  kHostOnly = 0b1u << 1,
  kHostToDevice = 0b1u << 2,
  kDeviceToHost = 0b1u << 3,
};

} // namespace clspvtest

#endif // CLSPV_TEST_CONFIG_HPP
