/*!
  \file vulkan_device.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_DEVICE_HPP
#define CLSPV_TEST_VULKAN_DEVICE_HPP

// Standard C++ library
#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
// Vulkan
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
// ClspvTest
#include "config.hpp"
#include "device_options.hpp"
#include "vulkan_physical_device_info.hpp"

namespace clspvtest {

// Forward declaration
template <typename> class VulkanBuffer;

/*!
  */
class VulkanDevice
{
 public:
  enum class VendorId : uint32b
  {
    kAmd = 0x1002, // AMD
    kImgTec = 0x1010, // ImgTec
    kNvidia = 0x10de, // NVIDIA
    kArm = 0x13b5, // ARM
    kQualcomm = 0x5143, // Qualcomm
    kIntel = 0x8086 // INTEL
  };


  //! Initialize a vulkan device
  VulkanDevice(DeviceOptions& options);

  //! Destroy a vulkan instance
  ~VulkanDevice() noexcept;


  //! Allocate a memory of a buffer
  template <typename Type>
  void allocate(const std::size_t size, VulkanBuffer<Type>* buffer) noexcept;

  //! Return the workgroup size for the work dimension
  template <std::size_t kDimension>
  std::array<uint32b, 3> calcWorkGroupSize(
      const std::array<uint32b, kDimension>& works) const noexcept;

  //! Return the command pool
  vk::CommandPool& commandPool(const QueueType queue_type) noexcept;

  //! Return the command pool
  const vk::CommandPool& commandPool(const QueueType queue_type) const noexcept;

  //! Deallocate a memory of a buffer
  template <typename Type>
  void deallocate(VulkanBuffer<Type>* buffer) noexcept;

  //! Destroy a vulkan instance
  void destroy() noexcept;

  //! Return the device body
  vk::Device& device() noexcept;

  //! Return the device body
  const vk::Device& device() const noexcept;

  //! Return the list of device info
//  static std::vector<VulkanPhysicalDeviceInfo> getPhysicalDeviceInfoList(
//      zisc::pmr::memory_resource* mem_resource =
//          zisc::SimpleMemoryResource::sharedResource()) noexcept;

  //! Return the shader module by the index
  const vk::ShaderModule& getShaderModule(const std::size_t index) const noexcept;

  //! Return the vendor name corresponding to the vendor ID
  static std::string getVendorName(const uint32b id) noexcept;

  //! Check if the device has the shader module
  bool hasShaderModule(const std::size_t index) const noexcept;

  //! Initialize local-work size
  void initLocalWorkSize(const uint32b subgroup_size) noexcept;

  //! Return the local-work size for the work dimension
  template <std::size_t kDimension>
  const std::array<uint32b, 3>& localWorkSize() const noexcept;

//  //! Make a kernel
//  template <std::size_t kDimension, typename Function, typename ...ArgumentTypes>
//  UniqueKernel<kDimension, ArgumentTypes...> makeKernel(
//      const uint32b module_index,
//      const std::string_view kernel_name) noexcept;

  //! Return the memory allocator of the device
  VmaAllocator& memoryAllocator() noexcept;

  //! Return the memory allocator of the device
  const VmaAllocator& memoryAllocator() const noexcept;

  //! Return the device name
  std::string_view name() const noexcept;

  //! Return the physical device info
  const VulkanPhysicalDeviceInfo& physicalDeviceInfo() const noexcept;

  //! Set a shader module
  void setShaderModule(const std::vector<uint32b>& spirv_code,
                       const std::size_t index);

  //! Return the subgroup size
  uint32b subgroupSize() const noexcept;

  //! Submit a command
  void submit(const QueueType queue_type,
              const uint32b queue_index,
              const vk::CommandBuffer& command) const noexcept;

  //! Return the vendor name
  std::string_view vendorName() const noexcept;

  //! Wait this thread until all commands in the device are completed
  void waitForCompletion() const noexcept;

  //! Wait this thread until all commands in the queues are completed
  void waitForCompletion(const QueueType queue_type) const noexcept;

  //! Wait this thread until all commands in the queue are completed
  void waitForCompletion(const QueueType queue_type,
                         const uint32b queue_index) const noexcept;

 private:
  //! Output a debug message
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* /* userData */);

  //! Find the index of the optimal queue familty
  uint32b findQueueFamily(const QueueType queue_type) const noexcept;

  //! Return a queue
  vk::Queue getQueue(const QueueType queue_type,
                     const uint32b queue_index) const noexcept;

  //! Initialize a command pool
  void initCommandPool();

  //! Initialize a debug messenger
  void initDebugMessenger() noexcept;

  //! Initialize a device
  void initDevice(const DeviceOptions& options);

  //! Initialize a memory allocator
  void initMemoryAllocator();

  //! Initialize a vulkan device
  void initialize(const DeviceOptions& options);

  //! Initialize a physical device
  void initPhysicalDevice(const DeviceOptions& options);

  //! Initialize a queue family index list
  void initQueueFamilyIndexList() noexcept;

  //! Make a vulkan instance
  static vk::Instance makeInstance(const vk::ApplicationInfo& app_info,
                                   const bool enable_validation_layers);

  //! Make an application info
  static vk::ApplicationInfo makeApplicationInfo(
      const char* app_name,
      const uint32b app_version_major,
      const uint32b app_version_minor,
      const uint32b app_version_patch) noexcept;

  //! Return an index of a queue family
  uint32b queueFamilyIndex(const QueueType queue_type) const noexcept;


  VulkanPhysicalDeviceInfo device_info_;
  std::vector<vk::ShaderModule> shader_module_list_;
  std::vector<vk::CommandPool> command_pool_list_;
  vk::ApplicationInfo app_info_;
  vk::Instance instance_;
  vk::DebugUtilsMessengerEXT debug_messenger_;
  vk::PhysicalDevice physical_device_;
  vk::Device device_;
  VmaAllocator allocator_ = VK_NULL_HANDLE;
  std::string vendor_name_;
  std::vector<uint32b> queue_family_index_list_;
  std::array<std::size_t, 2> queue_family_index_ref_list_;
  std::array<std::array<uint32b, 3>, 3> local_work_size_list_;
};

// type aliases
using UniqueDevice = std::unique_ptr<VulkanDevice>;

} // namespace clspvtest

#include "vulkan_device-inl.hpp"

#endif // CLSPV_TEST_VULKAN_DEVICE_HPP
