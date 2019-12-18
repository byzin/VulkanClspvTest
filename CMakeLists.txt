# file: CMakeLists.txt
# author: Sho Ikeda
#
# Copyright (c) 2015-2019 Sho Ikeda
# This software is released under the MIT License.
# http://opensource.org/licenses/mit-license.php
# 

cmake_minimum_required(VERSION 3.12)

function(checkSubmodule submodule_path)
  if(NOT EXISTS ${submodule_path})
    get_filename_component(submodule_name "${submodule_path}" NAME)
    message(FATAL_ERROR "Submodule '${submodule_name}' not found, please init submodules.")
  endif()
endfunction(checkSubmodule)

# Set project information
project(VulkanClspvTest
        VERSION 1.0.0
        DESCRIPTION "Introduction to Vulkan Clspv."
        LANGUAGES C CXX)

# Configure build types
set(CMAKE_CONFIGURATION_TYPES "Debug" "RelWithDebInfo" "Release")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build configuration
include(${PROJECT_SOURCE_DIR}/cmake/platform.cmake)
initPlatform(platform_definitions)
#
include(${PROJECT_SOURCE_DIR}/cmake/compiler.cmake)
initCompilerOption()
getCxxCompilerFlags(cxx_compiler_flags cxx_linker_flags cxx_definitions)

# Thread
find_package(Threads REQUIRED)
# cpu_features
set(cpu_features_path ${PROJECT_SOURCE_DIR}/deps/cpu_features)
set(cpu_features_dst_path ${PROJECT_BINARY_DIR}/cpu_features)
checkSubmodule(${cpu_features_path})
file(MAKE_DIRECTORY ${cpu_features_dst_path})
add_subdirectory(${cpu_features_path} ${cpu_features_dst_path})
# lodepng
set(lodepng_dir ${PROJECT_SOURCE_DIR}/deps/lodepng)
checkSubmodule(${lodepng_dir})
include(${PROJECT_SOURCE_DIR}/cmake/lodepng.cmake)
loadLodepng(${lodepng_dir} lodepng_include_dir lodepng_library)
# Vulkan
find_package(Vulkan REQUIRED)
set(vma_include_dir ${PROJECT_SOURCE_DIR}/deps/VulkanMemoryAllocator/src)

# clspv test
file(COPY ${PROJECT_SOURCE_DIR}/table.png DESTINATION ${PROJECT_BINARY_DIR})
include(${PROJECT_SOURCE_DIR}/test/config.cmake)
buildVulkanClspvTest()
