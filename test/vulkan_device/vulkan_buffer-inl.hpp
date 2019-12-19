/*!
  \file vulkan_buffer-inl.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_BUFFER_INL_HPP
#define CLSPV_TEST_VULKAN_BUFFER_INL_HPP

#include "vulkan_buffer.hpp"
// Standard C++ library
#include <cstddef>
// Vulkan
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
// ClspvTest
#include "config.hpp"
#include "vulkan_device.hpp"

namespace clspvtest {

/*!
  */
template <typename T> inline
VulkanBuffer< T>::VulkanBuffer(const VulkanDevice* device,
                               const BufferUsage usage_flag) :
    device_{device},
    usage_flag_{usage_flag}
{
  initialize();
}

/*!
  */
template <typename T> inline
VulkanBuffer<T>::VulkanBuffer(const VulkanDevice* device,
                              const BufferUsage usage_flag,
                              const std::size_t size) :
    VulkanBuffer(device, usage_flag)
{
  setSize(size);
}

/*!
  */
template <typename T> inline
VulkanBuffer<T>::~VulkanBuffer() noexcept
{
  destroy();
}

/*!
  */
template <typename T> inline
VmaAllocationInfo& VulkanBuffer<T>::allocationInfo() noexcept
{
  return alloc_info_;
}

/*!
  */
template <typename T> inline
const VmaAllocationInfo& VulkanBuffer<T>::allocationInfo() const noexcept
{
  return alloc_info_;
}

/*!
  */
template <typename T> inline
vk::Buffer& VulkanBuffer<T>::buffer() noexcept
{
  return buffer_;
}

/*!
  */
template <typename T> inline
const vk::Buffer& VulkanBuffer<T>::buffer() const noexcept
{
  return buffer_;
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::copyTo(VulkanBuffer* dst,
                             const std::size_t count,
                             const std::size_t src_offset,
                             const std::size_t dst_offset,
                             const uint32b queue_index) const noexcept
{
  const std::size_t s = sizeof(Type) * count;
  const std::size_t src_offset_size = sizeof(Type) * src_offset;
  const std::size_t dst_offset_size = sizeof(Type) * dst_offset;
  const vk::BufferCopy copy_info{src_offset_size, dst_offset_size, s};

  vk::CommandBufferBeginInfo begin_info{};
  begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  copy_command_.begin(begin_info);

  copy_command_.copyBuffer(buffer(), dst->buffer(), 1, &copy_info);

  copy_command_.end();
  device_->submit(QueueType::kTransfer, queue_index, copy_command_);
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::destroy() noexcept
{
  if (buffer_) {
    auto d = const_cast<VulkanDevice*>(device_);
    d->deallocate(this);
  }
}

/*!
  */
template <typename T> inline
bool VulkanBuffer<T>::isDeviceMemory() const noexcept
{
  const auto& info = device_->physicalDeviceInfo();
  const auto& memory_property = info.memoryProperties().properties1_;
  const uint32b index = allocationInfo().memoryType;
  const auto flag = memory_property.memoryTypes[index].propertyFlags;
  const bool result = (flag & vk::MemoryPropertyFlagBits::eDeviceLocal) ==
                      vk::MemoryPropertyFlagBits::eDeviceLocal;
  return result;
}

/*!
  */
template <typename T> inline
bool VulkanBuffer<T>::isHostMemory() const noexcept
{
  const bool result = !isDeviceMemory();
  return result;
}

/*!
  */
template <typename T> inline
bool VulkanBuffer<T>::isHostVisible() const noexcept
{
  const auto& info = device_->physicalDeviceInfo();
  const auto& memory_property = info.memoryProperties().properties1_;
  const uint32b index = allocationInfo().memoryType;
  const auto flag = memory_property.memoryTypes[index].propertyFlags;
  const bool result = (flag & vk::MemoryPropertyFlagBits::eHostVisible) ==
                      vk::MemoryPropertyFlagBits::eHostVisible;
  return result;
}

/*!
  */
template <typename T> inline
auto VulkanBuffer<T>::mapMemory() noexcept -> MappedMemory<Type>
{
  using MappedMem = MappedMemory<Type>;
  typename MappedMem::ConstBufferP p = isHostVisible() ? this : nullptr;
  MappedMem memory{p};
  return memory;
}

/*!
  */
template <typename T> inline
auto VulkanBuffer<T>::mapMemory() const noexcept -> MappedMemory<ConstType>
{
  using MappedMem = MappedMemory<ConstType>;
  typename MappedMem::ConstBufferP p = isHostVisible() ? this : nullptr;
  MappedMem memory{p};
  return memory;
}

/*!
  */
template <typename T> inline
VmaAllocation& VulkanBuffer<T>::memory() noexcept
{
  return memory_;
}

/*!
  */
template <typename T> inline
const VmaAllocation& VulkanBuffer<T>::memory() const noexcept
{
  return memory_;
}

/*!
  */
template <typename T> inline
std::size_t VulkanBuffer<T>::memoryUsage() const noexcept
{
  const auto memory_usage = static_cast<std::size_t>(alloc_info_.size);
  return memory_usage;
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::read(Pointer data,
                           const std::size_t count,
                           const std::size_t offset,
                           const uint32b queue_index) const noexcept
{
  if (isHostVisible()) {
    auto src = this->mapMemory();
    const std::size_t s = sizeof(Type) * count;
    std::memcpy(data, src.data() + offset, s);
  }
  else {
    VulkanBuffer dst{device_, BufferUsage::kHostOnly};
    dst.setSize(count);
    copyTo(&dst, count, offset, 0, queue_index);
    device_->waitForCompletion(QueueType::kTransfer, queue_index);
    dst.read(data, count, 0, queue_index);
  }
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::setSize(const std::size_t size) noexcept
{
  destroy();
  size_ = size;
  auto d = const_cast<VulkanDevice*>(device_);
  d->allocate(size, this);
}

/*!
  */
template <typename T> inline
std::size_t VulkanBuffer<T>::size() const noexcept
{
  return size_;
}

/*!
  */
template <typename T> inline
BufferUsage VulkanBuffer<T>::usage() const noexcept
{
  return usage_flag_;
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::write(ConstPointer data,
                            const std::size_t count,
                            const std::size_t offset,
                            const uint32b queue_index) noexcept
{
  if (isHostVisible()) {
    auto dst = this->mapMemory();
    const std::size_t s = sizeof(Type) * count;
    std::memcpy(dst.data() + offset, data, s);
  }
  else {
    VulkanBuffer src{device_, BufferUsage::kHostOnly};
    src.setSize(count);
    src.write(data, count, 0, queue_index);
    src.copyTo(this, count, 0, offset, queue_index);
    device_->waitForCompletion(QueueType::kTransfer, queue_index);
  }
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::initialize()
{
  alloc_info_.size = 0;
  // Initialize a copy command
  const vk::CommandBufferAllocateInfo alloc_info{
      device_->commandPool(QueueType::kTransfer),
      vk::CommandBufferLevel::ePrimary,
      1};
  auto copy_commands = device_->device().allocateCommandBuffers(alloc_info);
  copy_command_ = copy_commands[0];
}

/*!
  */
template <typename T> inline
auto VulkanBuffer<T>::mappedMemory() const noexcept -> Pointer
{
  void* d = nullptr;
  const auto result = vmaMapMemory(device_->memoryAllocator(), memory_, &d);
  (void)result;
  return static_cast<Pointer>(d);
}

/*!
  */
template <typename T> inline
void VulkanBuffer<T>::unmapMemory() const noexcept
{
  vmaUnmapMemory(device_->memoryAllocator(), memory_);
}

} // namespace clspvtest

#endif // CLSPV_TEST_VULKAN_BUFFER_INL_HPP
