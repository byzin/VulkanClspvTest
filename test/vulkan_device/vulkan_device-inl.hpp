/*!
  \file vulkan_device-inl.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_DEVICE_INL_HPP
#define CLSPV_TEST_VULKAN_DEVICE_INL_HPP

#include "vulkan_device.hpp"
// Standard C++ library
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
// Vulkan
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
// ClspvTest
#include "config.hpp"
#include "device_options.hpp"
#include "vulkan_buffer.hpp"

namespace clspvtest {

/*!
  */
inline
VulkanDevice::VulkanDevice(DeviceOptions& options) :
    queue_family_index_ref_list_{{std::numeric_limits<uint32b>::max(),
                                  std::numeric_limits<uint32b>::max()}}
{
  initialize(options);
}

/*!
  */
inline
VulkanDevice::~VulkanDevice() noexcept
{
  destroy();
}

/*!
  */
template <typename Type> inline
void VulkanDevice::allocate(const std::size_t size,
                            VulkanBuffer<Type>* buffer) noexcept
{
  auto& b = buffer->buffer();
  auto& memory = buffer->memory();
  auto& alloc_info = buffer->allocationInfo();

  vk::BufferCreateInfo buffer_create_info;
  buffer_create_info.size = sizeof(Type) * size;
  buffer_create_info.usage = vk::BufferUsageFlagBits::eTransferSrc |
                             vk::BufferUsageFlagBits::eTransferDst;
  buffer_create_info.usage = buffer_create_info.usage | 
                             vk::BufferUsageFlagBits::eStorageBuffer;
  buffer_create_info.queueFamilyIndexCount =
      static_cast<uint32b>(queue_family_index_list_.size());
  buffer_create_info.pQueueFamilyIndices = queue_family_index_list_.data();

  VmaAllocationCreateInfo alloc_create_info;
  switch (buffer->usage()) {
   case BufferUsage::kHostOnly: {
    alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    break;
   }
   case BufferUsage::kHostToDevice: {
    alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    break;
   }
   case BufferUsage::kDeviceToHost: {
    alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
    break;
   }
   case BufferUsage::kDeviceOnly:
   default: {
    alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    break;
   }
  }
  alloc_create_info.flags = 0;
  alloc_create_info.requiredFlags = 0;
  alloc_create_info.preferredFlags = 0;
  alloc_create_info.memoryTypeBits = 0;
  alloc_create_info.pool = VK_NULL_HANDLE;
  alloc_create_info.pUserData = nullptr;

  const auto result = vmaCreateBuffer(
      allocator_,
      &static_cast<const VkBufferCreateInfo&>(buffer_create_info),
      &alloc_create_info,
      reinterpret_cast<VkBuffer*>(&b),
      &memory,
      &alloc_info);
  //! \todo Handle error
  if (result != VK_SUCCESS) {
  }
}

/*!
  */
template <std::size_t kDimension> inline
std::array<uint32b, 3> VulkanDevice::calcWorkGroupSize(
    const std::array<uint32b, kDimension>& works) const noexcept
{
  std::array<uint32b, 3> work_group_size{{1, 1, 1}};
  const auto& local_work_size = localWorkSize<kDimension>();
  for (std::size_t i = 0; i < kDimension; ++i) {
    work_group_size[i] = ((works[i] % local_work_size[i]) == 0)
        ? works[i] / local_work_size[i]
        : works[i] / local_work_size[i] + 1;
  }
  return work_group_size;
}

/*!
  */
inline
auto VulkanDevice::commandPool(const QueueType queue_type) noexcept
    -> vk::CommandPool&
{
  const std::size_t list_index = static_cast<std::size_t>(queue_type);
  const std::size_t ref_index = queue_family_index_ref_list_[list_index];
  return command_pool_list_[ref_index];
}

/*!
  */
inline
auto VulkanDevice::commandPool(const QueueType queue_type) const noexcept
    -> const vk::CommandPool&
{
  const std::size_t list_index = static_cast<std::size_t>(queue_type);
  const std::size_t ref_index = queue_family_index_ref_list_[list_index];
  return command_pool_list_[ref_index];
}

/*!
  */
template <typename Type> inline
void VulkanDevice::deallocate(VulkanBuffer<Type>* buffer) noexcept
{
  auto& b = buffer->buffer();
  auto& memory = buffer->memory();
  auto& alloc_info = buffer->allocationInfo();
  if (b) {
    vmaDestroyBuffer(allocator_, *reinterpret_cast<VkBuffer*>(&b), memory);
    b = nullptr;
    memory = VK_NULL_HANDLE;
    alloc_info.size = 0;
  }
}

/*!
  */
inline
void VulkanDevice::destroy() noexcept
{
  if (device_) {
    for (auto& module : shader_module_list_) {
      if (module) {
        device_.destroyShaderModule(module);
        module = nullptr;
      }
    }
    if (allocator_)  {
      vmaDestroyAllocator(allocator_);
      allocator_ = VK_NULL_HANDLE;
    }
    for (std::size_t i = 0; i < command_pool_list_.size(); ++i) {
      auto command_pool = command_pool_list_[i];
      if (command_pool) {
        device_.destroyCommandPool(command_pool);
        command_pool_list_[i] = nullptr;
      }
    }
    device_.destroy();
    device_ = nullptr;
  }

  if (debug_messenger_) {
    auto destroyDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            instance_.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
    destroyDebugUtilsMessengerEXT(
        static_cast<VkInstance>(instance_),
        static_cast<VkDebugUtilsMessengerEXT>(debug_messenger_),
        nullptr);
    debug_messenger_ = nullptr;
  }

  if (instance_) {
    instance_.destroy();
    instance_ = nullptr;
  }
}

/*!
  */
inline
vk::Device& VulkanDevice::device() noexcept
{
  return device_;
}

/*!
  */
inline
const vk::Device& VulkanDevice::device() const noexcept
{
  return device_;
}

/*!
  */
//inline
//auto VulkanDevice::getPhysicalDeviceInfoList(
//    zisc::pmr::memory_resource* mem_resource) noexcept
//        -> std::vector<VulkanPhysicalDeviceInfo>
//{
//  const vk::ApplicationInfo app_info = makeApplicationInfo("", 0, 0, 0);
//  auto instance = makeInstance(app_info, true);
//  std::vector<VulkanPhysicalDeviceInfo> device_info_list;
//  if (instance) {
//    auto [result, physical_device_list] = instance.enumeratePhysicalDevices();
//    if (result == vk::Result::eSuccess) {
//      device_info_list.reserve(physical_device_list.size());
//      for (std::size_t i = 0; i < physical_device_list.size(); ++i) {
//        const auto& device = physical_device_list[i];
//        device_info_list.emplace_back(mem_resource);
//        device_info_list[i].fetch(device);
//      }
//    }
//  }
//  instance.destroy();
//  return device_info_list;
//}

/*!
  */
inline
const vk::ShaderModule& VulkanDevice::getShaderModule(
    const std::size_t index) const noexcept
{
  return shader_module_list_[index];
}

/*!
  */
inline
std::string VulkanDevice::getVendorName(const uint32b id) noexcept
{
  using namespace std::string_literals;
  std::string vendor_name;
  switch (id) {
   case static_cast<uint32b>(VendorId::kAmd): {
    vendor_name = "AMD"s;
    break;
   }
   case static_cast<uint32b>(VendorId::kImgTec): {
    vendor_name = "ImgTec"s;
    break;
   }
   case static_cast<uint32b>(VendorId::kNvidia): {
    vendor_name = "NVIDIA"s;
    break;
   }
   case static_cast<uint32b>(VendorId::kArm): {
    vendor_name = "ARM"s;
    break;
   }
   case static_cast<uint32b>(VendorId::kQualcomm): {
    vendor_name = "Qualcomm"s;
    break;
   }
   case static_cast<uint32b>(VendorId::kIntel): {
    vendor_name = "INTEL"s;
    break;
   }
   default: {
    vendor_name = "N/A"s;
    break;
   }
  }
  return vendor_name;
}

/*!
  */
inline
bool VulkanDevice::hasShaderModule(const std::size_t index) const noexcept
{
  const bool flag = (index < shader_module_list_.size()) &&
                    shader_module_list_[index];
  return flag;
}

/*!
  */
inline
void VulkanDevice::initLocalWorkSize(const uint32b subgroup_size) noexcept
{
  for (uint32b dim = 1; dim <= local_work_size_list_.size(); ++dim) {
    std::array<uint32b, 3> local_work_size{{1, 1, 1}};
    const auto product = [](const std::array<uint32b, 3>& s)
    {
      return std::accumulate(s.begin(), s.end(), 1u, std::multiplies<uint32b>());
    };
    for (uint32b i = 0; product(local_work_size) < subgroup_size; i = (i + 1) % dim)
      local_work_size[i] *= 2;
    local_work_size_list_[dim - 1] = local_work_size;
  }
}

/*!
  */
template <std::size_t kDimension> inline
const std::array<uint32b, 3>& VulkanDevice::localWorkSize() const noexcept
{
  static_assert((0 < kDimension) && (kDimension <= 3),
                "The dimension is out of range.");
  return local_work_size_list_[kDimension - 1];
}

///*!
//  */
//template <std::size_t kDimension, typename Function, typename ...ArgumentTypes>
//inline
//UniqueKernel<kDimension, ArgumentTypes...> VulkanDevice::makeKernel(
//    const uint32b module_index,
//    const std::string_view kernel_name) noexcept
//{
//  using UniqueVulkanKernel = zisc::UniqueMemoryPointer<VulkanKernel<
//      kDimension,
//      Function,
//      ArgumentTypes...>>;
//  UniqueKernel<kDimension, ArgumentTypes...> kernel =
//      UniqueVulkanKernel::make(memoryResource(), this, module_index, kernel_name);
//  return kernel;
//}

/*!
  */
inline
VmaAllocator& VulkanDevice::memoryAllocator() noexcept
{
  return allocator_;
}

/*!
  */
inline
const VmaAllocator& VulkanDevice::memoryAllocator() const noexcept
{
  return allocator_;
}

/*!
  */
inline
std::string_view VulkanDevice::name() const noexcept
{
  const auto& device_info = physicalDeviceInfo();
  std::string_view device_name{device_info.properties().properties1_.deviceName};
  return device_name;
}

/*!
  */
inline
auto VulkanDevice::physicalDeviceInfo() const noexcept
    -> const VulkanPhysicalDeviceInfo&
{
  return device_info_;
}

/*!
  */
inline
void VulkanDevice::setShaderModule(const std::vector<uint32b>& spirv_code,
                                   const std::size_t index)
{
  if (shader_module_list_.size() <= index)
    shader_module_list_.resize(index + 1);
  else if (hasShaderModule(index))
    device_.destroyShaderModule(getShaderModule(index));

  static_assert(sizeof(uint32b) == 4, "The size of uint32b isn't 4 bytes.");
  const vk::ShaderModuleCreateInfo create_info{vk::ShaderModuleCreateFlags{},
                                               4 * spirv_code.size(),
                                               spirv_code.data()};
  vk::ShaderModule shader_module = device_.createShaderModule(create_info);
  shader_module_list_[index] = shader_module;
}

/*!
  */
inline
uint32b VulkanDevice::subgroupSize() const noexcept
{
  return local_work_size_list_[0][0];
}

/*!
  */
inline
void VulkanDevice::submit(const QueueType queue_type,
                          const uint32b queue_index,
                          const vk::CommandBuffer& command) const noexcept
{
  vk::Queue q = getQueue(queue_type, queue_index);
  const vk::SubmitInfo info{0, nullptr, nullptr, 1, &command};
  q.submit(1, &info, nullptr);
}

/*!
  */
inline
std::string_view VulkanDevice::vendorName() const noexcept
{
  std::string_view vendor_name{vendor_name_};
  return vendor_name;
}

/*!
  */
inline
void VulkanDevice::waitForCompletion() const noexcept
{
  device_.waitIdle();
}

/*!
  */
inline
void VulkanDevice::waitForCompletion(const QueueType queue_type) const noexcept
{
  const uint32b family_index = queueFamilyIndex(queue_type);
  const auto& info = physicalDeviceInfo();
  const auto& family_info_list = info.queueFamilyPropertiesList();
  const auto& family_info = family_info_list[family_index].properties1_;
  for (uint32b i = 0; i < family_info.queueCount; ++i)
    waitForCompletion(queue_type, i);
}

/*!
  */
inline
void VulkanDevice::waitForCompletion(const QueueType queue_type,
                                     const uint32b queue_index) const noexcept
{
  vk::Queue q = getQueue(queue_type, queue_index);
  q.waitIdle();
}

/*!
  */
inline
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDevice::debugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* /* user_data */)
{
  constexpr std::size_t max_add_message_length = 1024;
  bool is_error = false;
  std::string message;

  {
    char prefix[64] = "";
    if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
      std::strcat(prefix, "VERBOSE : ");
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
      std::strcat(prefix, "INFO : ");
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      std::strcat(prefix, "WARNING : ");
    }
    else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      std::strcat(prefix, "ERROR : ");
      is_error = true;
    }

    if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
      std::strcat(prefix, "GENERAL");
    }
    else if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
      std::strcat(prefix, "VALIDATION");
    }
    else if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
      std::strcat(prefix, "PERF");
    }

    char tmp_message[max_add_message_length];
    std::sprintf(tmp_message, "%s - Message ID Number %d, Message ID Name %s :\n%s",
        prefix,
        callback_data->messageIdNumber,
        callback_data->pMessageIdName,
        callback_data->pMessage);
    message += tmp_message;
  }

  if (0 < callback_data->objectCount) {
    char tmp_message[max_add_message_length];
    std::sprintf(tmp_message, "\n Objects - %d\n", callback_data->objectCount);
    for (std::size_t object = 0; object < callback_data->objectCount; ++object) {
      std::sprintf(tmp_message, "%s Object[%d] - Type %s, Value %p, Name \"%s\"\n",
          tmp_message,
          static_cast<int>(object),
          vk::to_string(static_cast<vk::ObjectType>(callback_data->pObjects[object].objectType)).data(),
          reinterpret_cast<void*>(callback_data->pObjects[object].objectHandle),
          callback_data->pObjects[object].pObjectName);
    }
    message += tmp_message;
  }

  if (0 < callback_data->cmdBufLabelCount) {
    char tmp_message[max_add_message_length];
    std::sprintf(tmp_message, "\n Command Buffer Labels - %d\n",
        callback_data->cmdBufLabelCount);
    for (std::size_t label = 0; label < callback_data->cmdBufLabelCount; ++label) {
      std::sprintf(tmp_message, "%s Label[%d] - %s { %lf, %lf, %lf, %lf}\n",
          tmp_message,
          static_cast<int>(label),
          callback_data->pCmdBufLabels[label].pLabelName,
          static_cast<double>(callback_data->pCmdBufLabels[label].color[0]),
          static_cast<double>(callback_data->pCmdBufLabels[label].color[1]),
          static_cast<double>(callback_data->pCmdBufLabels[label].color[2]),
          static_cast<double>(callback_data->pCmdBufLabels[label].color[3]));
    }
    message += tmp_message;
  }

  std::ostream* output = (is_error) ? &std::cerr : &std::cout;
  (*output) << message << std::endl;

  return VK_FALSE;
}

/*!
  */
inline
uint32b VulkanDevice::findQueueFamily(const QueueType queue_type) const noexcept
{
  const auto& queue_family_list = physicalDeviceInfo().queueFamilyPropertiesList();

  uint32b index = std::numeric_limits<uint32b>::max();
  bool is_found = false;

  const vk::QueueFlagBits target = (queue_type == QueueType::kCompute)
      ? vk::QueueFlagBits::eCompute
      : vk::QueueFlagBits::eTransfer;
  const vk::QueueFlagBits other = (queue_type == QueueType::kCompute)
      ? vk::QueueFlagBits::eTransfer
      : vk::QueueFlagBits::eCompute;

  auto has_flag = [](const vk::QueueFamilyProperties& family,
                     const vk::QueueFlagBits flag)
  {
    return (family.queueFlags & flag) == flag;
  };

  for (std::size_t i = 0; !is_found && (i < queue_family_list.size()); ++i) {
    const auto& family = queue_family_list[i].properties1_;
    if (!has_flag(family, vk::QueueFlagBits::eGraphics) &&
        !has_flag(family, other) && has_flag(family, target)) {
      index = static_cast<uint32b>(i);
      is_found = true;
    }
  }

  for (std::size_t i = 0; !is_found && (i < queue_family_list.size()); ++i) {
    const auto& family = queue_family_list[i].properties1_;
    if (!has_flag(family, other) && has_flag(family, target)) {
      index = static_cast<uint32b>(i);
      is_found = true;
    }
  }

  for (std::size_t i = 0; !is_found && (i < queue_family_list.size()); ++i) {
    const auto& family = queue_family_list[i].properties1_;
    if (!has_flag(family, vk::QueueFlagBits::eGraphics) &&
        has_flag(family, target)) {
      index = static_cast<uint32b>(i);
      is_found = true;
    }
  }

  for (std::size_t i = 0; !is_found && (i < queue_family_list.size()); ++i) {
    const auto& family = queue_family_list[i].properties1_;
    if (has_flag(family, target)) {
      index = static_cast<uint32b>(i);
      is_found = true;
    }
  }

  return index;
}

/*!
  */
inline
vk::Queue VulkanDevice::getQueue(const QueueType queue_type,
                                 const uint32b queue_index) const noexcept
{
  const uint32b family_index = queueFamilyIndex(queue_type);
  const auto& info = physicalDeviceInfo();
  const auto& family_info_list = info.queueFamilyPropertiesList();
  const auto& family_info = family_info_list[family_index].properties1_;
  const uint32b index = queue_index % family_info.queueCount;
  auto q = device_.getQueue(family_index, index);
  return q;
}

/*!
  */
inline
void VulkanDevice::initCommandPool()
{
  command_pool_list_.reserve(queue_family_index_list_.size());
  for (std::size_t i = 0; i < queue_family_index_list_.size(); ++i) {
    const vk::CommandPoolCreateInfo pool_info{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        queue_family_index_list_[i]};
    vk::CommandPool command_pool = device_.createCommandPool(pool_info);
    command_pool_list_.emplace_back(command_pool);
  }
}

/*!
  */
inline
void VulkanDevice::initDebugMessenger() noexcept
{
  auto createDebugUtilsMessengerEXT =
      reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          instance_.getProcAddr("vkCreateDebugUtilsMessengerEXT"));

  VkDebugUtilsMessengerCreateInfoEXT create_info;
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.messageSeverity =
//      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = &VulkanDevice::debugMessengerCallback;
  create_info.pUserData = nullptr;

  vk::DebugUtilsMessengerEXT debug_messenger;
  createDebugUtilsMessengerEXT(
      static_cast<VkInstance>(instance_),
      &create_info,
      nullptr,
      reinterpret_cast<VkDebugUtilsMessengerEXT*>(&debug_messenger));
  debug_messenger_ = debug_messenger;
}

/*!
  */
inline
void VulkanDevice::initDevice(const DeviceOptions& options)
{
  std::vector<const char*> layers{};
  const std::vector<const char*> extensions{{
      VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
      VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
      VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
      VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,
      VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
      VK_KHR_VARIABLE_POINTERS_EXTENSION_NAME}};

  if (options.enable_debug_) {
    layers.emplace_back("VK_LAYER_KHRONOS_validation");
  }

  const auto& info = physicalDeviceInfo();
  vk::PhysicalDeviceFeatures device_features;
  {
    const auto& features = info.features().features1_;
    device_features.shaderFloat64 = features.shaderFloat64;
    device_features.shaderInt64 = features.shaderInt64;
    device_features.shaderInt16 = features.shaderInt16;
  }

  std::vector<std::vector<float>> priority_list;
  priority_list.reserve(queue_family_index_list_.size());
  std::vector<vk::DeviceQueueCreateInfo> queue_create_info_list;
  queue_create_info_list.reserve(queue_family_index_list_.size());
  for (auto family_index : queue_family_index_list_) {
    const auto& family_info_list = info.queueFamilyPropertiesList();
    const auto& family_info = family_info_list[family_index].properties1_;
    priority_list.emplace_back();
    priority_list.back().resize(family_info.queueCount, 0.0f);
    queue_create_info_list.emplace_back(vk::DeviceQueueCreateFlags{},
                                        family_index,
                                        family_info.queueCount,
                                        priority_list.back().data());
  }

  vk::DeviceCreateInfo device_create_info{
      vk::DeviceCreateFlags{},
      static_cast<uint32b>(queue_create_info_list.size()),
      queue_create_info_list.data(),
      static_cast<uint32b>(layers.size()),
      layers.data(),
      static_cast<uint32b>(extensions.size()),
      extensions.data(),
      &device_features};

  // features2
  auto b16bit_storage_feature = info.features().b16bit_storage_;
  auto b8bit_storage_feature = info.features().b8bit_storage_;
  auto float16_int8_feature = info.features().float16_int8_;
  auto variable_pointers_feature = info.features().variable_pointers_;
  VulkanPhysicalDeviceInfo::link(device_create_info,
                                 b16bit_storage_feature,
                                 b8bit_storage_feature,
                                 float16_int8_feature,
                                 variable_pointers_feature);

  vk::Device device = physical_device_.createDevice(device_create_info);
  device_ = device;
}

/*!
  */
inline
void VulkanDevice::initMemoryAllocator()
{
  VmaAllocatorCreateInfo allocator_create_info{};
  allocator_create_info.flags = 0;
  allocator_create_info.physicalDevice = physical_device_;
  allocator_create_info.device = device_;
  allocator_create_info.preferredLargeHeapBlockSize = 0;
  allocator_create_info.pAllocationCallbacks = nullptr;
  allocator_create_info.pDeviceMemoryCallbacks = nullptr;
  allocator_create_info.frameInUseCount = 0;
  allocator_create_info.pHeapSizeLimit = nullptr;
  allocator_create_info.pVulkanFunctions = nullptr;
  allocator_create_info.pRecordSettings = nullptr;

  auto result = vmaCreateAllocator(&allocator_create_info, &allocator_);
  //! \todo Handle error
  if (result != VK_SUCCESS) {
  }
}

/*!
  */
inline
void VulkanDevice::initialize(const DeviceOptions& options)
{
  app_info_ = makeApplicationInfo(options.app_name_,
                                  options.app_version_major_,
                                  options.app_version_minor_,
                                  options.app_version_patch_);
  instance_ = makeInstance(app_info_, options.enable_debug_);
  if (options.enable_debug_)
    initDebugMessenger();
  initPhysicalDevice(options);
  initQueueFamilyIndexList();

  {
    const auto& info = physicalDeviceInfo();
    uint32b subgroup_size = info.properties().subgroup_.subgroupSize;
    subgroup_size = ((1 <= subgroup_size) && (subgroup_size <= 128))
        ? subgroup_size
        : 32;
    vendor_name_ = getVendorName(info.properties().properties1_.vendorID);
    initLocalWorkSize(subgroup_size);
  }

  initDevice(options);
  initCommandPool();
  initMemoryAllocator();
}

/*!
  */
inline
void VulkanDevice::initPhysicalDevice(const DeviceOptions& options)
{
  const auto physical_device_list = instance_.enumeratePhysicalDevices();
  physical_device_ = physical_device_list[options.vulkan_device_number_];
  device_info_.fetch(physical_device_);
}

/*!
  */
inline
void VulkanDevice::initQueueFamilyIndexList() noexcept
{
  std::array<uint32b, 2> index_list{{findQueueFamily(QueueType::kCompute),
                                     findQueueFamily(QueueType::kTransfer)}};
  queue_family_index_list_.reserve(index_list.size());
  for (std::size_t i = 0; i < index_list.size(); ++i) {
    const uint32b family_index = index_list[i];
    auto ite = std::find(queue_family_index_list_.begin(),
                         queue_family_index_list_.end(),
                         family_index);
    if (ite != queue_family_index_list_.end()) {
      const auto ref_index = std::distance(queue_family_index_list_.begin(), ite);
      queue_family_index_ref_list_[i] = static_cast<std::size_t>(ref_index);
    }
    else {
      queue_family_index_list_.emplace_back(family_index);
      queue_family_index_ref_list_[i] = i;
    }
  }
}

/*!
  */
inline
vk::Instance VulkanDevice::makeInstance(const vk::ApplicationInfo& app_info,
                                        const bool enable_validation_layers)
{
  std::vector<const char*> layers{};
  std::vector<const char*> extensions{
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
  };

  if (enable_validation_layers) {
    layers.emplace_back("VK_LAYER_KHRONOS_validation");
    extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  const vk::InstanceCreateInfo createInfo{vk::InstanceCreateFlags{},
                                          &app_info,
                                          static_cast<uint32b>(layers.size()),
                                          layers.data(),
                                          static_cast<uint32b>(extensions.size()),
                                          extensions.data()};
  vk::Instance instance = vk::createInstance(createInfo);

  return instance;
}

/*!
  */
inline
vk::ApplicationInfo VulkanDevice::makeApplicationInfo(
    const char* app_name,
    const uint32b app_version_major,
    const uint32b app_version_minor,
    const uint32b app_version_patch) noexcept
{
  const uint32b app_version = VK_MAKE_VERSION(app_version_major,
                                              app_version_minor,
                                              app_version_patch);
  static constexpr char engine_name[] = "ClspvTest";
  constexpr uint32b engine_version = VK_MAKE_VERSION(1, 0, 0);
  constexpr uint32b api_version = VK_API_VERSION_1_1;
  const vk::ApplicationInfo app_info{app_name,
                                     app_version,
                                     engine_name,
                                     engine_version,
                                     api_version};
  return app_info;
}

/*!
  */
inline
uint32b VulkanDevice::queueFamilyIndex(const QueueType queue_type) const noexcept
{
  const std::size_t list_index = static_cast<std::size_t>(queue_type);
  const std::size_t ref_index = queue_family_index_ref_list_[list_index];
  const uint32b family_index = queue_family_index_list_[ref_index];
  return family_index;
}

} // namespace clspvtest

#endif // CLSPV_TEST_VULKAN_DEVICE_INL_HPP
