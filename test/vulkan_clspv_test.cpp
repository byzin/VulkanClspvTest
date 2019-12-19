/*!
  \file vulkan_clspv_test.cpp
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
// cpu_features
#include "cpu_features_macros.h"
// lodepng
#include "lodepng.h"
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
  using clspvtest::int8b;
  using clspvtest::uint8b;
  using clspvtest::int16b;
  using clspvtest::uint16b;
  using clspvtest::int32b;
  using clspvtest::uint32b;
  using clspvtest::int64b;
  using clspvtest::uint64b;

  std::cout << "Blur an image with gaussian kernel." << std::endl;

  clspvtest::UniqueDevice device;
  clspvtest::DeviceOptions device_options;
  device_options.app_name_ = "VulkanClspvTest";
  device_options.app_version_major_ = 1;
  device_options.app_version_minor_ = 0;
  device_options.app_version_patch_ = 0;
  device_options.vulkan_device_number_ = 0; //!< Use 0th GPU

  {
    // Load an input image
    std::vector<uint8b> image;
    std::array<uint32b, 2> res{{0, 0}};
    {
      const char* name = "table.png";
      std::cout << "- Load `" << name << "'." << std::endl;
      const uint32b error = lodepng::decode(image, res[0], res[1], name, LCT_RGB);
      if (error) {
        //! \todo Handle error
      }
    }
    const uint32b w = res[0];
    const uint32b h = res[1];

    // Run a vulkan kernel
    std::cout << "- Run a gaussian kernel." << std::endl;
    using clspvtest::BufferUsage;
    bool success = true;
    clspvtest::UniqueKernel<1, uint8b, uint8b, uint32b, uint32b> kernel;
    clspvtest::UniqueBuffer<uint8b> buffer1;
    clspvtest::UniqueBuffer<uint8b> buffer2;
    clspvtest::UniqueBuffer<uint32b> block_size;
    clspvtest::UniqueBuffer<uint32b> resolution;
    try {
      // Create a vulkan device
      device = std::make_unique<clspvtest::VulkanDevice>(device_options);
      {
        const std::string info = getDeviceInfo(*device);
        std::cout << info << std::endl;
      }
      // Create vulkan buffers
      buffer1 = makeBuffer<clspvtest::uint8b>(device.get(),
                                              BufferUsage::kDeviceOnly);
      buffer1->setSize(3 * w * h);
      buffer1->write(image.data(), image.size(), 0, 0);
      buffer2 = makeBuffer<clspvtest::uint8b>(device.get(),
                                              BufferUsage::kDeviceOnly);
      buffer2->setSize(3 * w * h);
      const uint32b bsize = 16;
      block_size = makeBuffer<clspvtest::uint32b>(device.get(),
                                                  BufferUsage::kHostToDevice);
      block_size->setSize(1);
      block_size->write(&bsize, 1, 0, 0);
      resolution = makeBuffer<clspvtest::uint32b>(device.get(),
                                                  BufferUsage::kHostToDevice);
      resolution->setSize(2);
      resolution->write(res.data(), res.size(), 0, 0);
      // Create a kernel
      kernel = makeKernel<1, uint8b, uint8b, uint32b, uint32b>(
          device.get(),
          "test_kernel.spv",
          0,
          "applyGaussianFilter");
      // Run the kernel 3 times
      kernel->run(*buffer1, *buffer2, *block_size, *resolution, {(w * h) / bsize}, 0);
      device->waitForCompletion();
      kernel->run(*buffer2, *buffer1, *block_size, *resolution, {(w * h) / bsize}, 0);
      device->waitForCompletion();
      kernel->run(*buffer1, *buffer2, *block_size, *resolution, {(w * h) / bsize}, 0);
      device->waitForCompletion();

      // Read the result
      buffer2->read(image.data(), image.size(), 0, 0);
    }
    catch (const std::exception& error) {
      std::cerr << "Error: " << error.what() << std::endl;
      success = false;
    }

    // Save the output image
    if (success) {
      const char* name = "result_gpu.png";
      std::cout << "- Save the result as `" << name << "'." << std::endl;
      const uint32b error = lodepng::encode(name, image, w, h, LCT_RGB);
      if (error) {
        //! \todo Handle error
      }
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
