/*!
  \file vulkan_kernel.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_KERNEL_HPP
#define CLSPV_TEST_VULKAN_KERNEL_HPP

// Standard C++ library
#include <array>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
// Vulkan
#include <vulkan/vulkan.hpp>
// ClspvTest
#include "config.hpp"

namespace clspvtest {

// Forward declaration
template <typename> class VulkanBuffer;
class VulkanDevice;

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes>
class VulkanKernel
{
 public:
  template <typename Type>
  using BufferRef = std::add_lvalue_reference_t<VulkanBuffer<Type>>;


  //! Construct a kernel
  VulkanKernel(VulkanDevice* device,
               const uint32b module_index,
               const std::string_view kernel_name);

  //! Destroy a kernel
  ~VulkanKernel() noexcept;


  //! Destroy a kernel
  void destroy() noexcept;

  //! Return an assigned device
  VulkanDevice* device() noexcept;

  //! Return an assigned device
  const VulkanDevice* device() const noexcept;

  //! Return the number of a kernel arguments
  static constexpr std::size_t numOfArguments() noexcept;

  //! Execute a kernel
  void run(BufferRef<ArgumentTypes>... args,
           const std::array<uint32b, kDimension> works,
           const uint32b queue_index);

  //! Return the workgroup dimension
  static constexpr std::size_t workgroupDimension() noexcept;

 private:
  //! Bind buffers
  void bindBuffers(BufferRef<ArgumentTypes>... args);

  //! Dispatch
  void dispatch(const std::array<uint32b, kDimension> works);

  //! Get the VkBuffer of the given buffer
  template <typename Type>
  vk::Buffer& getVkBuffer(Type& buffer) const noexcept;

  //! Initialize a command buffer
  void initCommandBuffer();

  //! Initialize a compute pipeline
  void initComputePipeline(const uint32b module_index,
                           const std::string_view kernel_name);

  //! Initialize a descriptor pool
  void initDescriptorPool();

  //! Initialize a descriptor set
  void initDescriptorSet();

  //! Initialize a descriptor set layout
  void initDescriptorSetLayout();

  //! Initialize a kernel
  void initialize(const uint32b module_index,
                  const std::string_view kernel_name);

  //! Initialize a pipeline layout
  void initPipelineLayout();

  //! Check if the current buffers are same as previous buffers
  bool isSameArgs(BufferRef<ArgumentTypes>... args) const noexcept;


  VulkanDevice* device_;
  vk::DescriptorSetLayout descriptor_set_layout_;
  vk::DescriptorPool descriptor_pool_;
  vk::DescriptorSet descriptor_set_;
  vk::PipelineLayout pipeline_layout_;
  vk::Pipeline compute_pipeline_;
  vk::CommandBuffer command_buffer_;
  std::array<vk::Buffer, sizeof...(ArgumentTypes)> buffer_list_;
};

// Type aliases
template <std::size_t kDimension, typename ...ArgumentTypes>
using UniqueKernel = std::unique_ptr<VulkanKernel<kDimension, ArgumentTypes...>>;

} // namespace clspvtest

#include "vulkan_kernel-inl.hpp"

#endif // CLSPV_TEST_VULKAN_KERNEL_HPP
