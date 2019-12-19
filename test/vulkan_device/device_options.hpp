/*!
  \file device_options.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_DEVICE_OPTIONS_HPP
#define CLSPV_TEST_DEVICE_OPTIONS_HPP

// Standard C++ library
#include <array>
#include <cstddef>
#include <string_view>
// ClspvTest
#include "config.hpp"

namespace clspvtest {

/*!
  */
struct DeviceOptions
{
  const char* app_name_ = "Application";
  uint32b app_version_major_ = 0;
  uint32b app_version_minor_ = 0;
  uint32b app_version_patch_ = 0;
  bool enable_debug_ = true;
  uint32b vulkan_device_number_ = 0;
};

} // namespace clspvtest

#endif // CLSPV_TEST_DEVICE_OPTIONS_HPP
