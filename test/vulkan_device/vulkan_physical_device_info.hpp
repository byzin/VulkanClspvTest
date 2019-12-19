/*!
  \file vulkan_physical_device_info.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_PHYSICAL_DEVICE_INFO_HPP
#define CLSPV_TEST_VULKAN_PHYSICAL_DEVICE_INFO_HPP

// Standard C++ library
#include <vector>
// Vulkan
#include <vulkan/vulkan.hpp>
// ClspvTest
#include "config.hpp"

namespace clspvtest {

/*!
  \brief Properties and features of a vulkan device
  */
class VulkanPhysicalDeviceInfo
{
 public:
  //! Extension properties of a device
  struct ExtensionProperties
  {
    vk::ExtensionProperties properties1_;
  };

  //! Layer properties of a device
  struct LayerProperties
  {
    vk::LayerProperties properties1_;
  };

  //! Properties of a device
  struct Properties
  {
    vk::PhysicalDeviceProperties properties1_;
    vk::PhysicalDeviceBlendOperationAdvancedPropertiesEXT blend_operation_advanced_;
    vk::PhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_;
    vk::PhysicalDeviceDepthStencilResolvePropertiesKHR depth_stencil_resolve_;
    vk::PhysicalDeviceDescriptorIndexingPropertiesEXT descriptor_indexing_;
    vk::PhysicalDeviceDiscardRectanglePropertiesEXT discard_rectangle_;
    vk::PhysicalDeviceDriverPropertiesKHR driver_;
    vk::PhysicalDeviceExternalMemoryHostPropertiesEXT external_memory_host_;
    vk::PhysicalDeviceFloatControlsPropertiesKHR float_controls_;
    vk::PhysicalDeviceFragmentDensityMapPropertiesEXT fragment_density_map_;
    vk::PhysicalDeviceIDProperties id_properties_;
    vk::PhysicalDeviceInlineUniformBlockPropertiesEXT inline_uniform_block_;
    vk::PhysicalDeviceMaintenance3Properties maintenance3_;
    vk::PhysicalDeviceMultiviewProperties multiview_;
    vk::PhysicalDevicePCIBusInfoPropertiesEXT pci_bus_info_;
    vk::PhysicalDevicePointClippingProperties point_clipping_;
    vk::PhysicalDeviceProtectedMemoryProperties protected_memory_;
    vk::PhysicalDevicePushDescriptorPropertiesKHR push_descriptor_;
    vk::PhysicalDeviceSampleLocationsPropertiesEXT sample_locations_;
    vk::PhysicalDeviceSamplerFilterMinmaxPropertiesEXT sampler_filter_minmax_;
    vk::PhysicalDeviceSubgroupProperties subgroup_;
    vk::PhysicalDeviceTransformFeedbackPropertiesEXT transform_feedback_;
    vk::PhysicalDeviceVertexAttributeDivisorPropertiesEXT vertex_attribute_divisor_;
  };

  //! Features of a device
  struct Features
  {
    vk::PhysicalDeviceFeatures features1_;
    vk::PhysicalDevice16BitStorageFeatures b16bit_storage_;
    vk::PhysicalDevice8BitStorageFeaturesKHR b8bit_storage_;
    vk::PhysicalDeviceASTCDecodeFeaturesEXT astc_decode_;
    vk::PhysicalDeviceBlendOperationAdvancedFeaturesEXT blend_operation_advanced_;
    vk::PhysicalDeviceBufferDeviceAddressFeaturesEXT buffer_device_address_;
    vk::PhysicalDeviceConditionalRenderingFeaturesEXT conditional_rendering_;
    vk::PhysicalDeviceDepthClipEnableFeaturesEXT depth_clip_enabled_;
    vk::PhysicalDeviceDescriptorIndexingFeaturesEXT descriptor_indexing_;
    vk::PhysicalDeviceFloat16Int8FeaturesKHR float16_int8_;
    vk::PhysicalDeviceFragmentDensityMapFeaturesEXT fragment_density_map_;
//    vk::PhysicalDeviceFragmentShaderInterlockFeaturesEXT fragment_shader_inter_lock_;
    vk::PhysicalDeviceHostQueryResetFeaturesEXT host_query_reset_;
    vk::PhysicalDeviceInlineUniformBlockFeaturesEXT inline_uniform_block_;
    vk::PhysicalDeviceMemoryPriorityFeaturesEXT memory_priority_features_;
    vk::PhysicalDeviceMultiviewFeatures multiview_;
    vk::PhysicalDeviceProtectedMemoryFeatures protected_memory_;
    vk::PhysicalDeviceSamplerYcbcrConversionFeatures sampler_ycbcr_conversion_;
    vk::PhysicalDeviceScalarBlockLayoutFeaturesEXT scalar_block_layout_;
    vk::PhysicalDeviceShaderAtomicInt64FeaturesKHR shader_atomic_int64_;
    vk::PhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_;
    vk::PhysicalDeviceTransformFeedbackFeaturesEXT transform_feedback_;
    vk::PhysicalDeviceUniformBufferStandardLayoutFeaturesKHR uniform_buffer_standard_layout_;
    vk::PhysicalDeviceVariablePointersFeatures variable_pointers_;
    vk::PhysicalDeviceVertexAttributeDivisorFeaturesEXT vertex_attribute_divisor_;
    vk::PhysicalDeviceVulkanMemoryModelFeaturesKHR vulkan_memory_mode_;
    vk::PhysicalDeviceYcbcrImageArraysFeaturesEXT ycbcr_image_arrays_;
  };

  //! Memory properties of a device
  struct MemoryProperties
  {
    vk::PhysicalDeviceMemoryProperties properties1_;
    vk::PhysicalDeviceMemoryBudgetPropertiesEXT budget_;
  };

  //! Queue family properties of a device
  struct QueueFamilyProperties
  {
    vk::QueueFamilyProperties properties1_;
  };


  //! Initialize device info
  VulkanPhysicalDeviceInfo() noexcept;

  //! Move a data
  VulkanPhysicalDeviceInfo(VulkanPhysicalDeviceInfo&& other) noexcept;


  //! Move a data
  VulkanPhysicalDeviceInfo& operator=(VulkanPhysicalDeviceInfo&& other) noexcept;


  //! Return extension properties list
  std::vector<ExtensionProperties>& extensionPropertiesList() noexcept;

  //! Return extension properties list
  const std::vector<ExtensionProperties>& extensionPropertiesList() const noexcept;

  //! Return features
  Features& features() noexcept;

  //! Return features
  const Features& features() const noexcept;

  //! Fetch device info from a physical device
  void fetch(const vk::PhysicalDevice& device);

  //! Return layer properties list
  std::vector<LayerProperties>& layerPropertiesList() noexcept;

  //! Return layer properties list
  const std::vector<LayerProperties>& layerPropertiesList() const noexcept;

  template <typename Type1, typename Type2, typename ...Types>
  static void link(Type1&& value1, Type2&& value2, Types&&... values) noexcept;

  //! Return memory properties
  MemoryProperties& memoryProperties() noexcept;

  //! Return memory properties
  const MemoryProperties& memoryProperties() const noexcept;

  //! Return properties
  Properties& properties() noexcept;

  //! Return properties
  const Properties& properties() const noexcept;

  //! Return queue family properties list
  std::vector<QueueFamilyProperties>& queueFamilyPropertiesList() noexcept;

  //! Return queue family properties list
  const std::vector<QueueFamilyProperties>& queueFamilyPropertiesList() const noexcept;

 private:
  //! Fetch extension properties from a physical device
  void fetchExtensionProperties(const vk::PhysicalDevice& device);

  //! Fetch features from a physical device
  void fetchFeatures(const vk::PhysicalDevice& device) noexcept;

  //! Fetch layer properties from a physical device
  void fetchLayerProperties(const vk::PhysicalDevice& device);

  //! Fetch memory properties from a physical device
  void fetchMemoryProperties(const vk::PhysicalDevice& device) noexcept;

  //! Fetch properties from a physical device
  void fetchProperties(const vk::PhysicalDevice& device) noexcept;

  //! Fetch queue family properties from a physical device
  void fetchQueueFamilyProperties(const vk::PhysicalDevice& device);


  std::vector<ExtensionProperties> extension_properties_list_;
  std::vector<LayerProperties> layer_properties_list_;
  std::vector<QueueFamilyProperties> queue_family_properties_list_;
  Properties properties_;
  Features features_;
  MemoryProperties memory_properties_;
};

} // namespace clspvtest

#include "vulkan_physical_device_info-inl.hpp"

#endif // CLSPV_TEST_VULKAN_PHYSICAL_DEVICE_INFO_HPP
