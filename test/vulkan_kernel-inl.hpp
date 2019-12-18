/*!
  \file vulkan_kernel-inl.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_KERNEL_INL_HPP
#define CLSPV_TEST_VULKAN_KERNEL_INL_HPP

#include "vulkan_kernel.hpp"
// Standard C++ library
#include <array>
#include <cstddef>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>
// Vulkan
#include <vulkan/vulkan.hpp>
// ClspvTest
#include "config.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_device.hpp"

namespace clspvtest {

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
VulkanKernel<kDimension, ArgumentTypes...>::VulkanKernel(
    VulkanDevice* device,
    const uint32b module_index,
    const std::string_view kernel_name) : device_{device}
{
  initialize(module_index, kernel_name);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
VulkanKernel<kDimension, ArgumentTypes...>::~VulkanKernel() noexcept
{
  destroy();
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::destroy() noexcept
{
  const auto& device = device_->device();
  if (compute_pipeline_) {
    device.destroyPipeline(compute_pipeline_, nullptr);
    compute_pipeline_ = nullptr;
  }
  if (pipeline_layout_) {
    device.destroyPipelineLayout(pipeline_layout_, nullptr);
    pipeline_layout_ = nullptr;
  }
  if (descriptor_pool_) {
    device.destroyDescriptorPool(descriptor_pool_, nullptr);
    descriptor_pool_ = nullptr;
  }
  if (descriptor_set_layout_) {
    device.destroyDescriptorSetLayout(descriptor_set_layout_, nullptr);
    descriptor_set_layout_ = nullptr;
  }
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
auto VulkanKernel<kDimension, ArgumentTypes...>::device() noexcept
    -> VulkanDevice*
{
  return device_;
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
auto VulkanKernel<kDimension, ArgumentTypes...>::device() const noexcept
    -> const VulkanDevice*
{
  return device_;
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
constexpr std::size_t VulkanKernel<kDimension, ArgumentTypes...>::
    numOfArguments() noexcept
{
  const std::size_t num_of_arguments = sizeof...(ArgumentTypes);
  return num_of_arguments;
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::run(
    BufferRef<ArgumentTypes>... args,
    const std::array<uint32b, kDimension> works,
    const uint32b queue_index)
{
  if (!isSameArgs(args...))
    bindBuffers(args...);
  dispatch(works);
  device()->submit(QueueType::kCompute, queue_index, command_buffer_);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
constexpr std::size_t VulkanKernel<kDimension, ArgumentTypes...>::
    workgroupDimension() noexcept
{
  return kDimension;
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::bindBuffers(
    BufferRef<ArgumentTypes>... args)
{
  constexpr std::size_t num_of_buffers = numOfArguments();
  if ((num_of_buffers == 0) || isSameArgs(args...))
    return;

  std::array<vk::Buffer, num_of_buffers> buffer_list{getVkBuffer(args)...};
  std::array<vk::DescriptorBufferInfo, num_of_buffers> descriptor_info_list;
  std::array<vk::WriteDescriptorSet, num_of_buffers> descriptor_set_list;

  for (std::size_t index = 0; index < num_of_buffers; ++index) {
    auto& descriptor_info = descriptor_info_list[index];
    descriptor_info.buffer = buffer_list[index];
    descriptor_info.offset = 0;
    descriptor_info.range = VK_WHOLE_SIZE;

    auto& descriptor_set = descriptor_set_list[index];
    descriptor_set.dstSet = descriptor_set_;
    descriptor_set.dstBinding = static_cast<uint32b>(index);
    descriptor_set.dstArrayElement = 0;
    descriptor_set.descriptorCount = 1;
    descriptor_set.descriptorType = vk::DescriptorType::eStorageBuffer;
    descriptor_set.pImageInfo = nullptr;
    descriptor_set.pBufferInfo = &descriptor_info;
    descriptor_set.pTexelBufferView = nullptr;
  }

  const auto& device = device_->device();
  device.updateDescriptorSets(static_cast<uint32b>(num_of_buffers),
                              descriptor_set_list.data(),
                              0,
                              nullptr);
  buffer_list_ = std::move(buffer_list);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::dispatch(
    std::array<uint32b, kDimension> works)
{
  const auto group_size = device_->calcWorkGroupSize(works);
  vk::CommandBufferBeginInfo begin_info{};
  begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  command_buffer_.begin(begin_info);

  command_buffer_.bindPipeline(vk::PipelineBindPoint::eCompute, compute_pipeline_);
  command_buffer_.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                     pipeline_layout_,
                                     0,
                                     1,
                                     &descriptor_set_,
                                     0,
                                     nullptr);
  command_buffer_.dispatch(group_size[0], group_size[1], group_size[2]);

  command_buffer_.end();
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes>
template <typename Type> inline
auto VulkanKernel<kDimension, ArgumentTypes...>::getVkBuffer(
    Type& buffer) const noexcept -> vk::Buffer&
{
  return buffer.buffer();
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initCommandBuffer()
{
  const vk::CommandBufferAllocateInfo alloc_info{
      device_->commandPool(QueueType::kCompute),
      vk::CommandBufferLevel::ePrimary,
      1};
  const auto& device = device_->device();
  auto command_buffers = device.allocateCommandBuffers(alloc_info);
  command_buffer_ = command_buffers[0];
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initComputePipeline(
    const uint32b module_index,
    const std::string_view kernel_name)
{
  // Set constant IDs
  constexpr std::size_t num_of_entries = 3u;
  std::array<uint32b, num_of_entries> constant_data;
  {
    const auto& local_work_size = device_->localWorkSize<kDimension>();
    for (std::size_t i = 0; i < local_work_size.size(); ++i)
      constant_data[i] = local_work_size[i];
  }
  std::array<vk::SpecializationMapEntry, num_of_entries> entries;
  for (std::size_t i = 0; i < entries.size(); ++i) {
    entries[i].constantID = static_cast<uint32b>(i);
    entries[i].offset = static_cast<uint32b>(i * sizeof(uint32b));
    entries[i].size = sizeof(uint32b);
  }
  const vk::SpecializationInfo info{static_cast<uint32b>(num_of_entries),  
                                    entries.data(),
                                    num_of_entries * sizeof(uint32b),
                                    constant_data.data()};
  // Shader stage create info
  const auto& shader_module = device_->getShaderModule(module_index);
  const vk::PipelineShaderStageCreateInfo shader_stage_create_info{
      vk::PipelineShaderStageCreateFlags{},
      vk::ShaderStageFlagBits::eCompute,
      shader_module,
      kernel_name.data(),
      &info};
  // Pipeline create info
  const vk::ComputePipelineCreateInfo create_info{
      vk::PipelineCreateFlags{},
      shader_stage_create_info,
      pipeline_layout_};

  const auto& device = device_->device();
  auto pipelines = device.createComputePipelines(vk::PipelineCache{},
                                                 create_info);
  compute_pipeline_ = pipelines[0];
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initDescriptorPool()
{
  vk::DescriptorPoolSize pool_size;
  pool_size.type = vk::DescriptorType::eStorageBuffer;
  pool_size.descriptorCount = static_cast<uint32b>(numOfArguments());
  const vk::DescriptorPoolCreateInfo create_info{vk::DescriptorPoolCreateFlags{},
                                                 1,
                                                 1,
                                                 &pool_size};
  const auto& device = device_->device();
  descriptor_pool_ = device.createDescriptorPool(create_info);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initDescriptorSet()
{
  const vk::DescriptorSetAllocateInfo alloc_info{descriptor_pool_,
                                                 1,
                                                 &descriptor_set_layout_};
  const auto& device = device_->device();
  auto descriptor_sets = device.allocateDescriptorSets(alloc_info);
  descriptor_set_ = descriptor_sets[0];
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initDescriptorSetLayout()
{
  constexpr std::size_t num_of_buffers = numOfArguments();
  std::array<vk::DescriptorSetLayoutBinding, num_of_buffers> layout_bindings;
  for (std::size_t index = 0; index < num_of_buffers; ++index) {
    layout_bindings[index] = vk::DescriptorSetLayoutBinding{
        static_cast<uint32b>(index),
        vk::DescriptorType::eStorageBuffer,
        1,
        vk::ShaderStageFlagBits::eCompute};
  }
  const vk::DescriptorSetLayoutCreateInfo create_info{
      vk::DescriptorSetLayoutCreateFlags{},
      static_cast<uint32b>(layout_bindings.size()),
      layout_bindings.data()};
  const auto& device = device_->device();
  descriptor_set_layout_ = device.createDescriptorSetLayout(create_info);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initialize(
    const uint32b module_index,
    const std::string_view kernel_name)
{
  initDescriptorSetLayout();
  initDescriptorPool();
  initDescriptorSet();
  initPipelineLayout();
  initComputePipeline(module_index, kernel_name);
  initCommandBuffer();
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
void VulkanKernel<kDimension, ArgumentTypes...>::initPipelineLayout()
{
  const vk::PipelineLayoutCreateInfo create_info{
      vk::PipelineLayoutCreateFlags{},
      1,
      &descriptor_set_layout_};
  const auto& device = device_->device();
  pipeline_layout_ = device.createPipelineLayout(create_info);
}

/*!
  */
template <std::size_t kDimension, typename ...ArgumentTypes> inline
bool VulkanKernel<kDimension, ArgumentTypes...>::isSameArgs(
    BufferRef<ArgumentTypes>... args) const noexcept
{
  constexpr std::size_t num_of_buffers = numOfArguments();
  std::array<vk::Buffer, num_of_buffers> buffer_list{{getVkBuffer(args)...}};
  bool result = true;
  for (std::size_t i = 0; (i < buffer_list.size()) && result; ++i)
    result = buffer_list_[i] == buffer_list[i];
  return result;
}

} // namespace clspvtest

#endif // CLSPV_TEST_VULKAN_KERNEL_INL_HPP
