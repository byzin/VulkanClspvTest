/*!
  \file vulkan_initialization.hpp
  \author Sho Ikeda
  \brief Initialize vulkan specific definitions. This header must be included only once in a project before any vulkan instances are created.

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_VULKAN_INITIALIZATION_HPP
#define CLSPV_TEST_VULKAN_INITIALIZATION_HPP

// Vulkan initialization

#if defined(VMA_IMPLEMENTATION)
static_assert(false, "The macro 'VMA_IMPLEMENTATION' is already defined.");
#endif // VMA_IMPLEMENTATION

#if defined(AMD_VULKAN_MEMORY_ALLOCATOR_H)
static_assert(false, "The header 'vk_mem_alloc.h' is already loaded.");
#endif // AMD_VULKAN_MEMORY_ALLOCATOR_H

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#endif // CLSPV_TEST_VULKAN_INITIALIZATION_HPP
