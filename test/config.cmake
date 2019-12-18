# file: config.cmake
# author: Sho Ikeda
#
# Copyright (c) 2015-2019 Sho Ikeda
# This software is released under the MIT License.
# http://opensource.org/licenses/mit-license.php
# 

set(__test_root__ ${CMAKE_CURRENT_LIST_DIR})


function(initTestOption)
endfunction(initTestOption)


function(getTestWarningFlags test_warning_flags)
  set(warning_flags "")

  #  if(ZINVUL_SUPPRESS_EXCESSIVE_WARNING)
  #    if(Z_CLANG)
  #      list(APPEND warning_flags -Wno-covered-switch-default
  #                                -Wno-exit-time-destructors
  #                                -Wno-extra-semi-stmt
  #                                -Wno-float-equal
  #                                -Wno-global-constructors
  #                                -Wno-old-style-cast
  #                                -Wno-padded
  #                                -Wno-undefined-reinterpret-cast
  #                                -Wno-weak-vtables
  #                                -Wno-zero-as-null-pointer-constant
  #                                )
  #    elseif(Z_GCC)
  #      list(APPEND warning_flags -Wno-restrict
  #                                -Wno-sign-conversion
  #                                -Wno-strict-overflow
  #                                -Wno-unused-but-set-variable
  #                                -Wno-redundant-move
  #                                )
  #    elseif(Z_MSVC)
  #      list(APPEND warning_flags /wd4996
  #                                )
  #    endif()
  #  endif()

  # Output variables
  set(${test_warning_flags} ${warning_flags} PARENT_SCOPE)
endfunction(getTestWarningFlags)

function(buildClModule module_name)
  set(cl_source_file ${PROJECT_SOURCE_DIR}/test/test_kernel.cl)
  set(spv_file_path ${PROJECT_BINARY_DIR}/test_kernel.spv)

  find_program(clspv "clspv")
  if(clspv-NOTFOUND)
    message(FATAL_ERROR "'clspv' not found in path.")
  endif()

  set(clspv_options --c++ --f16bit_storage --inline-entry-points --int8 -O=3)
  add_custom_command(OUTPUT ${spv_file_path}
    COMMAND ${clspv} ${clspv_options} -o=${spv_file_path} ${cl_source_file}
    DEPENDS ${cl_source_file}
    COMMENT "Building CL object ${cl_source_file}")
  add_custom_target(${module_name} DEPENDS ${spv_file_path})
endfunction(buildClModule)

function(buildVulkanClspvTest)
  initTestOption()

  buildClModule(test_kernel)

  set(test_definitions VULKAN_HPP_TYPESAFE_CONVERSION
                       VULKAN_HPP_NO_SMART_HANDLE)
  if(Z_DEBUG_MODE)
    list(APPEND test_definitions VMA_DEBUG_INITIALIZE_ALLOCATIONS=1
                                 VMA_DEBUG_MARGIN=32
                                 VMA_DEBUG_DETECT_CORRUPTION=1)
  endif()

  set(test_name VulkanClspvTest)
  # Build unit tests
  add_executable(${test_name} ${PROJECT_SOURCE_DIR}/test/vulkan_clspv_test.cpp)
  # Set unittest properties
  set_target_properties(${test_name} PROPERTIES CXX_STANDARD 17
                                                CXX_EXTENSIONS ON
                                                CXX_STANDARD_REQUIRED ON)
  getCxxWarningFlags(cxx_warning_flags)
  getTestWarningFlags(test_warning_flags)
  target_compile_options(${test_name} PRIVATE ${cxx_compiler_flags}
                                              ${cxx_warning_flags}
                                              ${test_warning_flags})
  target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/test)
  target_include_directories(${test_name} SYSTEM PRIVATE ${vma_include_dir}
                                                         ${lodepng_include_dir})
  target_link_libraries(${test_name} PRIVATE Vulkan::Vulkan
                                             ${lodepng_library}
                                             cpu_features
                                             ${cxx_linker_flags}
                                             ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(${test_name} PRIVATE ${test_definitions}
                                                  ${cxx_definitions}
                                                  ${platform_definitions})
  add_dependencies(${test_name} test_kernel)
endfunction(buildVulkanClspvTest)
