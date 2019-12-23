/*!
  \file vulkan_clspv_test1.cpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

// Standard C++ library
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
// ClspvTest
#include "vulkan_device/vulkan_initialization.hpp" //!< Initialize vulkan memory allocator. This header must be included only once in a project before any vulkan instances are created.
#include "vulkan_device/config.hpp"
#include "vulkan_device/device_options.hpp"
#include "vulkan_device/vulkan_buffer.hpp"
#include "vulkan_device/vulkan_kernel.hpp"
#include "vulkan_device/vulkan_device.hpp"

// Forward declaration
std::string getDeviceInfo(const clspvtest::VulkanDevice& device);

std::vector<clspvtest::uint32b> loadModuleSpirvCode(
    const std::string_view module_file_name);
template <typename Type>

clspvtest::UniqueBuffer<Type> makeBuffer(
    clspvtest::VulkanDevice* device,
    const clspvtest::BufferUsage usage_flag);
template <std::size_t kDimension, typename ...ArgumentTypes>

clspvtest::UniqueKernel<kDimension, ArgumentTypes...> makeKernel(
    clspvtest::VulkanDevice* device,
    const std::string_view module_file_name,
    const clspvtest::uint32b module_index,
    const std::string_view kernel_name);


int main(int /* argc */, char** /* argv */)
{
  using clspvtest::uint8b;
  using clspvtest::uint32b;

  clspvtest::UniqueDevice device;
  clspvtest::DeviceOptions device_options;
  device_options.app_name_ = "VulkanClspvTest1";
  device_options.app_version_major_ = 1;
  device_options.app_version_minor_ = 0;
  device_options.app_version_patch_ = 0;
  device_options.vulkan_device_number_ = 0; //!< Use 0th GPU
#if defined(Z_DEBUG_MODE)
  device_options.enable_debug_ = true;
#else
  device_options.enable_debug_ = false;
#endif

  std::array<float, 7> results;
  {
    using clspvtest::BufferUsage;
    bool success = true;
    clspvtest::UniqueKernel<1, float, float> kernel;
    clspvtest::UniqueBuffer<float> buffer1;
    clspvtest::UniqueBuffer<float> buffer2;
    try {
      // Create a vulkan device
      device = std::make_unique<clspvtest::VulkanDevice>(device_options);
      {
        const std::string info = getDeviceInfo(*device);
        std::cout << info << std::endl;
      }
      // Create vulkan buffers
      buffer1 = makeBuffer<float>(device.get(), BufferUsage::kDeviceOnly);
      buffer1->setSize(sizeof(float) * 16); // sizeof(Marix4x4)
      buffer2 = makeBuffer<float>(device.get(), BufferUsage::kDeviceOnly);
      buffer2->setSize(results.size());
      // Create a kernel
      kernel = makeKernel<1, float, float>(
          device.get(),
          "vulkan_clspv_test1.spv",
          0,
          "testSummation");
      const uint32b num_threads = 1;
      kernel->run(*buffer1, *buffer2, {num_threads}, 0);
      device->waitForCompletion();

      // Read the result
      buffer2->read(results.data(), results.size(), 0, 0);
    }
    catch (const std::exception& error) {
      std::cerr << "Error: " << error.what() << std::endl;
      success = false;
    }

    // Save the output image
    if (success) {
      for (std::size_t i = 0; i < results.size(); ++i)
        std::cout << "  output[" << i << "] = " << results[i] << std::endl;
    }
  }

  return 0;
}

std::string getDeviceInfo(const clspvtest::VulkanDevice& device)
{
  using namespace std::string_literals;
  std::string info;
  info = "    Vulkan Device:\n"s;
  info += "      Vendor: "s + device.vendorName().data() + "\n"s;
  info += "      Name: "s + device.name().data() + "\n"s;
  info += "      Subgroup: "s + std::to_string(device.subgroupSize());
  return info;
}

std::vector<clspvtest::uint32b> loadModuleSpirvCode(
    const std::string_view module_file_name)
{
  static_assert(sizeof(clspvtest::uint32b) == 4,
                "The size of uint32b isn't 4 bytes.");
  std::vector<clspvtest::uint32b> spirv_code{};
  std::ifstream spirv_file{module_file_name.data(), std::ios_base::binary};
  std::streamsize spirv_size = 0;
  {
    const auto begin = spirv_file.tellg();
    spirv_file.seekg(0, std::ios_base::end);
    const auto end = spirv_file.tellg();
    spirv_size = end - begin;
    if ((spirv_size % 4) != 0) {
      //! \todo Handle error
    }
    spirv_file.clear();
    spirv_file.seekg(0, std::ios_base::beg);
  }
  spirv_code.resize(static_cast<std::size_t>(spirv_size / 4));
  spirv_file.read(reinterpret_cast<char*>(spirv_code.data()), spirv_size);
  return spirv_code;
}

/*!
  \brief Make a buffer
  */
template <typename Type>
clspvtest::UniqueBuffer<Type> makeBuffer(
    clspvtest::VulkanDevice* device,
    const clspvtest::BufferUsage usage_flag)
{
  using Buffer = clspvtest::VulkanBuffer<Type>;
  auto buffer = std::make_unique<Buffer>(device, usage_flag);
  return buffer;
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes>
clspvtest::UniqueKernel<kDimension, ArgumentTypes...> makeKernel(
    clspvtest::VulkanDevice* device,
    const std::string_view module_file_name,
    const clspvtest::uint32b module_index,
    const std::string_view kernel_name)
{
  if (!device->hasShaderModule(module_index)) {
    const std::vector<clspvtest::uint32b> spirv_code =
        loadModuleSpirvCode(module_file_name);
    device->setShaderModule(spirv_code, module_index);
  }
  using Kernel = clspvtest::VulkanKernel<kDimension, ArgumentTypes...>;
  auto kernel = std::make_unique<Kernel>(device, module_index, kernel_name);
  return kernel;
}
