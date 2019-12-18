/*!
  \file test_kernel.cl
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */


// Type aliases
typedef char int8b;
typedef short int16b;
typedef int int32b;
typedef long int64b;
typedef unsigned char uint8b;
typedef unsigned short uint16b;
typedef unsigned int uint32b;
typedef unsigned long uint64b;


__kernel void applyGaussianFilter(__global const uint8b* inputs,
                                  __global uint8b* outputs,
                                  __global uint* block_size,
                                  const uint2 resolution)
{
  const uint bsize = block_size[0];
  const uint n = (resolution.x * resolution.y) / bsize;
  const uint index = get_global_id(0);
  if (n <= index)
    return;

  for (uint b = 0; b < block_size[0]; ++b) {
    const uint pixel_index = bsize * index + b;
    const uint x = pixel_index % resolution.x;
    const uint y = pixel_index / resolution.x;

    constexpr uint weight_factors[] = {20, 15, 6, 1};
    float3 pixel = float3{0.0f, 0.0f, 0.0f};
    for (int j = -3; j < 4; ++j) {
      for (int i = -3; i < 4; ++i) {
        const int pixel_x = static_cast<int>(x) + i;
        const int pixel_y = static_cast<int>(y) + j;
        if ((0 <= pixel_x) && (pixel_x < resolution.x) &&
            (0 <= pixel_y) && (pixel_y < resolution.y)) {
          const uint w = weight_factors[abs(i)] * weight_factors[abs(j)];
          const float weight = static_cast<float>(w) / 4096.0f;
          const uint pixel_i = pixel_y * resolution.x + pixel_x;
          pixel.x += weight * static_cast<float>(inputs[3 * pixel_i]);
          pixel.y += weight * static_cast<float>(inputs[3 * pixel_i + 1]);
          pixel.z += weight * static_cast<float>(inputs[3 * pixel_i + 2]);
        }
      }
    }

    pixel = clamp(pixel, 0.0f, 255.0f);
    outputs[3 * pixel_index    ] = static_cast<uint8b>(pixel.x); 
    outputs[3 * pixel_index + 1] = static_cast<uint8b>(pixel.y); 
    outputs[3 * pixel_index + 2] = static_cast<uint8b>(pixel.z); 
  }
}
