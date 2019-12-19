/*!
  \file vulkan_buffer.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_BUFFER_HPP
#define CLSPV_TEST_VULKAN_BUFFER_HPP

// Standard C++ library
#include <cstddef>
#include <memory>
#include <type_traits>
// Vulkan
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"
// ClspvTest
#include "config.hpp"
#include "mapped_memory.hpp"

namespace clspvtest {

// Forward declaration
class VulkanDevice;

/*!
  */
template <typename T>
class VulkanBuffer
{
 public:
  //! The type of the buffer. "const", "volatile" and "reference" are removed
  using Type = std::remove_cv_t<std::remove_reference_t<T>>;
  using ConstType = std::add_const_t<Type>;
  using Pointer = std::add_pointer_t<Type>;
  using ConstPointer = std::add_pointer_t<ConstType>;


  //! Create an empty buffer
  VulkanBuffer(const VulkanDevice* device,
               const BufferUsage usage_flag);

  //! Create a buffer
  VulkanBuffer(const VulkanDevice* device,
               const BufferUsage usage_flag,
               const std::size_t size);

  //! Destroy a buffer
  ~VulkanBuffer() noexcept;


  //! Return the allocation info
  VmaAllocationInfo& allocationInfo() noexcept;

  //! Return the allocation info
  const VmaAllocationInfo& allocationInfo() const noexcept;

  //! Return the buffer body
  vk::Buffer& buffer() noexcept;

  //! Return the buffer body
  const vk::Buffer& buffer() const noexcept;

  //! Copy this buffer to a dst buffer
  void copyTo(VulkanBuffer* dst,
              const std::size_t count,
              const std::size_t src_offset,
              const std::size_t dst_offset,
              const uint32b queue_index) const noexcept;

  //! Destroy a buffer
  void destroy() noexcept;

  //! Check if a buffer memory is on device
  bool isDeviceMemory() const noexcept;

  //! Check if a buffer memory is on host
  bool isHostMemory() const noexcept;

  //! Check if a buffer memory is host visible
  bool isHostVisible() const noexcept;

  //! Map a buffer memory to a host
  MappedMemory<Type> mapMemory() noexcept;

  //! Map a buffer memory to a host
  MappedMemory<ConstType> mapMemory() const noexcept;

  //! Return the memory allocation
  VmaAllocation& memory() noexcept;

  //! Return the memory allocation
  const VmaAllocation& memory() const noexcept;

  //! Return the memory usage
  std::size_t memoryUsage() const noexcept;

  //! Read a data from a buffer
  void read(Pointer data,
            const std::size_t count,
            const std::size_t offset,
            const uint32b queue_index) const noexcept;

  //! Set a size of a buffer
  void setSize(const std::size_t size) noexcept;

  //! Return a size of a buffer
  std::size_t size() const noexcept;

  //! Return the usage flag
  BufferUsage usage() const noexcept;

  //! Write a data to a buffer
  void write(ConstPointer data,
             const std::size_t count,
             const std::size_t offset,
             const uint32b queue_index) noexcept;

 private:
  friend MappedMemory<Type>;
  friend MappedMemory<ConstType>;


  //! Initialize a buffer
  void initialize();

  //! Map a buffer memory to a host
  Pointer mappedMemory() const noexcept;

  //! Unmap a buffer memory
  void unmapMemory() const noexcept;


  const VulkanDevice* device_;
  vk::Buffer buffer_;
  vk::CommandBuffer copy_command_;
  VmaAllocation memory_ = VK_NULL_HANDLE;
  VmaAllocationInfo alloc_info_;
  BufferUsage usage_flag_;
  std::size_t size_ = 0;
};

// Type aliases
template <typename Type>
using UniqueBuffer = std::unique_ptr<VulkanBuffer<Type>>;

} // namespace clspvtest

#include "vulkan_buffer-inl.hpp"

#endif // CLSPV_TEST_VULKAN_BUFFER_HPP
