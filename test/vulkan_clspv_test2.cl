/*!
  \file vulkan_clspv_test2.cl
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-c++11-compat-pedantic"

// Type aliases
typedef char int8b;
typedef short int16b;
typedef int int32b;
typedef long int64b;
typedef unsigned char uint8b;
typedef unsigned short uint16b;
typedef unsigned int uint32b;
typedef unsigned long uint64b;


/*!
  \brief Apply 7x7 gaussian filter
  */
__kernel void applyGaussianFilter(__global const uint8b* inputs,
                                  __global uint8b* outputs,
                                  __global const uint* block_size,
                                  const uint2 resolution)
{
  const uint bsize = block_size[0];
  const uint n = (resolution.x * resolution.y) / bsize;
  const uint index = get_global_id(0);
  if (n <= index)
    return;

  constexpr __constant uint weight_factors[] = {20, 15, 6, 1};
  for (uint b = 0; b < block_size[0]; ++b) {
    const uint center_index = bsize * index + b;
    const uint center_x = center_index % resolution.x;
    const uint center_y = center_index / resolution.x;

    float3 value = float3{0.0f, 0.0f, 0.0f};
    for (int j = -3; j < 4; ++j) {
      for (int i = -3; i < 4; ++i) {
        const int x = static_cast<int>(center_x) + i;
        const int y = static_cast<int>(center_y) + j;
        if ((0 <= x) && (x < resolution.x) && (0 <= y) && (y < resolution.y)) {
          const uint w = weight_factors[abs(i)] * weight_factors[abs(j)];
          const float weight = static_cast<float>(w) / 4096.0f;
          const int pixel_index = y * static_cast<int>(resolution.x) + x;
          const uchar3 pixel = vload3(static_cast<size_t>(pixel_index), inputs);
          value += weight * convert_float3(pixel);
        }
      }
    }
    value = clamp(value, 0.0f, 255.0f);
    const uchar3 pixel = convert_uchar3(value);
    vstore3(pixel, center_index, outputs);
  }
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop
