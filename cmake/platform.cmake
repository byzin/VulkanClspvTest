# file: platform.cmake
# author: Sho Ikeda
# 
# Copyright (c) 2015-2019 Sho Ikeda
# This software is released under the MIT License.
# http://opensource.org/licenses/mit-license.php
# 


# Set boolean value option
macro(setBooleanOption variable value doc_string)
  set(${variable} ${value} CACHE BOOL ${doc_string})
endmacro(setBooleanOption)


# Set string value option
macro(setStringOption variable value doc_string)
  set(${variable} ${value} CACHE STRING ${doc_string})
endmacro(setStringOption)


# Initialize platform specific variables and get compiler flags
function(initPlatform platform_definitions)
  set(definitions "")

  # Detect Platform
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(Z_WINDOWS ON PARENT_SCOPE)
    set(system_definition Z_WINDOWS)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(Z_LINUX ON PARENT_SCOPE)
    set(system_definition Z_LINUX)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(Z_MAC ON PARENT_SCOPE)
    set(system_definition Z_MAC)
  else()
    set(unsupported_system ON)
  endif()
  list(APPEND definitions ${system_definition})
  if(unsupported_system)
    message(WARNING "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
  else()
    message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
  endif()

  # Detect C++ compiler
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(Z_GCC ON PARENT_SCOPE)
    set(compiler_definition Z_GCC)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(Z_CLANG ON PARENT_SCOPE)
    set(compiler_definition Z_CLANG)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(Z_CLANG ON PARENT_SCOPE)
    set(compiler_definition Z_CLANG Z_APPLE_CLANG)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(Z_MSVC ON PARENT_SCOPE)
    set(compiler_definition Z_MSVC)
  else()
    set(unsupported_compiler ON)
  endif()
  list(APPEND definitions ${compiler_definition})
  if(unsupported_compiler)
    message(WARNING "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
  else()
    message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()

  # Detect environment
  if(CMAKE_GENERATOR MATCHES ".*Makefiles")
    set(Z_MAKEFILE ON PARENT_SCOPE)
    set(generator_definition Z_MAKEFILE)
  elseif(CMAKE_GENERATOR MATCHES "Visual Studio.*")
    set(Z_VISUAL_STUDIO ON PARENT_SCOPE)
    set(generator_definition Z_VISUAL_STUDIO)
  elseif(CMAKE_GENERATOR MATCHES "Xcode")
    set(Z_XCODE ON PARENT_SCOPE)
    set(generator_definition "Z_XCODE")
  else()
    set(unsupported_generator ON)
  endif()
  list(APPEND definitions ${generator_definition})
  if(unsupported_generator)
    message(WARNING "Unsupported generator: ${CMAKE_GENERATOR}")
  else()
    message(STATUS "Generator: ${CMAKE_GENERATOR}")
  endif()


  # Detect build type
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(Z_DEBUG_MODE ON PARENT_SCOPE)
    set(build_mode_definition Z_DEBUG_MODE)
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(Z_DEBUG_MODE ON PARENT_SCOPE)
    set(Z_RELEASE_MODE ON PARENT_SCOPE)
    set(build_mode_definition Z_DEBUG_MODE Z_RELEASE_MODE)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(Z_RELEASE_MODE ON PARENT_SCOPE)
    set(build_mode_definition Z_RELEASE_MODE)
  else()
    set(unsupported_build_type ON)
  endif()
  list(APPEND definitions ${build_mode_definition})
  if(unsupported_build_type)
    message(WARNING "Unsupported build type: ${CMAKE_BUILD_TYPE}")
  else()
    message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")
  endif()


  # Output variable
  set(${platform_definitions} ${definitions} PARENT_SCOPE)
endfunction(initPlatform)
