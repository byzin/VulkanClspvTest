# VulkanClspvTest #

Introduction to Vulkan Clspv.

## Build instructions ##

### Prerequisites ###

* C++17 support compiler
* [CMake](https://cmake.org/download/) 3.12 or later
* [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) 1.1.126.0 or later
* [Clspv](https://github.com/google/clspv) ef5ba2b or later

### Dependencies ###

Download submodules

```sh
% git submodule update --init
```

### Windows ###

Create a build directory and run CMake.

```sh
% cmake -G"Visual Studio 15 2017" -Ax64 -Thost=x64 -DCMAKE_BUILD_TYPE=Release ..
% cmake --build . --config Release --parallel 8
```

### Linux ###

Make sure the script `setup-env.sh` in the Vulkan SDK is executed,
and the environment variable `VULKAN_SDK` points to the location of the Vulkan SDK,
```sh
% source ${path-to-vulkan-sdk}/setup-env.sh
% echo ${VULKAN_SDK}
${path-to-vulkan-sdk}/x86_64
```

Create a build directory and run CMake.

```sh
% cmake -DCMAKE_BUILD_TYPE=Release ..
% cmake --build . --config Release --parallel 8
```

### macOS ###

Make sure the environment variable `VULKAN_SDK` points to the location of the Vulkan SDK.
You can use `setup-mac-vulkan-env.sh`,

```sh
% cp setup-mac-vulkan-env.sh ${path-to-vulkan-sdk}/setup-env.sh
% source ${path-to-vulkan-sdk}/setup-env.sh
% echo ${VULKAN_SDK}
${path-to-vulkan-sdk}/macOS
```

Create a build directory and run CMake.

```sh
% cmake -DCMAKE_BUILD_TYPE=Release ..
% cmake --build . --config Release --parallel 8
```
