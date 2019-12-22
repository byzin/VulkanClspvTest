/*!
  \file vulkan_clspv_test1.cl
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-compat"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++98-c++11-compat-pedantic"

namespace clspvtest {

// Forward declaration
float sum(const float x);
float sum(const float4 x);
template <typename... Types> float sum(const Types... args);
class Matrix4x4;
Matrix4x4 operator+(const Matrix4x4& lhs, const Matrix4x4& rhs);

float sum(const float x)
{
  return x;
}

float sum(const float4 x)
{
  const float s = x.x + x.y + x.z + x.w;
  return s;
}

template <typename... Types>
float sum(const Types... args)
{
  // Clspv can build C++17 features
  if constexpr (0 < sizeof...(Types)) {
    const auto s = (args + ...);
    return sum(s);
  }
  else {
    return 0.0f;
  }
}

class Matrix4x4
{
 public:
  Matrix4x4() {}

  Matrix4x4(const float4 r1, const float4 r2, const float4 r3, const float4 r4) :
      r1_{r1}, r2_{r2}, r3_{r3}, r4_{r4}
  {
  }

  //! Return the sum of the elements
  float sum() const
  {
    const float s = clspvtest::sum(r1_, r2_, r3_, r4_);
    return s;
  }

  float4 r1_ = float4{0.0f, 0.0f, 0.0f, 0.0f},
         r2_ = float4{0.0f, 0.0f, 0.0f, 0.0f},
         r3_ = float4{0.0f, 0.0f, 0.0f, 0.0f},
         r4_ = float4{0.0f, 0.0f, 0.0f, 0.0f};
};

//! Add corresponding elements of the given two matrices
Matrix4x4 operator+(const Matrix4x4& lhs, const Matrix4x4& rhs)
{
  const Matrix4x4 result{lhs.r1_ + rhs.r1_,
                         lhs.r2_ + rhs.r2_,
                         lhs.r3_ + rhs.r3_,
                         lhs.r4_ + rhs.r4_};
  return result;
}

} // namespace clspvtest

__kernel void testSummation(__global clspvtest::Matrix4x4* value,
                            __global float* outputs)
{
  const uint index = get_global_id(0);
  if (0 < index)
    return;

  // Check clspvtest::Matrix4x4
  static_assert(sizeof(clspvtest::Matrix4x4) == 64,
                "The size of clspvtest::Matrix4x4 isn't 64 bytes.");
  static_assert(alignof(clspvtest::Matrix4x4) == 16,
                "The alignment of clspvtest::Matrix4x4 isn't 16 bytes.");

  // Constant value
  __constant constexpr float k = 4.0f; // constant value must be in the outermost scope of a kernel
  static_assert(k == 4.0f);

  outputs[0] = clspvtest::sum();
  outputs[1] = clspvtest::sum(sqrt(k)); // Use OpenCL C style built-in function
  outputs[2] = clspvtest::sum(1.0f, 2.0f, 3.0f);

  // Matrix4x4 in registers
  {
    __private const clspvtest::Matrix4x4 m;
    outputs[3] = m.sum();
  }
  {
    __private const clspvtest::Matrix4x4 m{float4{k, k, k, k},
                                           float4{k, k, k, k},
                                           float4{k, k, k, k},
                                           float4{k, k, k, k}};
    outputs[4] = m.sum();
  }

  // Matrix4x4 in global memory
  {
    __private const clspvtest::Matrix4x4 m1{float4{k, k, k, k},
                                            float4{k, k, k, k},
                                            float4{k, k, k, k},
                                            float4{k, k, k, k}};
    __private const clspvtest::Matrix4x4 m2{float4{0.0f, 1.0f, 2.0f, 3.0f},
                                            float4{4.0f, 5.0f, 6.0f, 7.0f},
                                            float4{8.0f, 9.0f, 10.0f, 11.0f},
                                            float4{12.0f, 13.0f, 14.0f, 15.0f}};
    value[0] = m1 + m2; // Initialize global Matrix4x4 
    outputs[5] = value->sum();
  }

  // Matrix4x4 in local memory
  __local clspvtest::Matrix4x4 storage[1]; // local variable must be in the outermost scope of a kernel
  {
    __private const clspvtest::Matrix4x4 m = value[0];
    storage[0] = m;
    outputs[6] = storage->sum();
  }
}

#pragma clang diagnostic pop
#pragma clang diagnostic pop
