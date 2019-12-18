/*!
  \file mapped_memory.hpp
  \author Sho Ikeda

  Copyright (c) 2015-2019 Sho Ikeda
  This software is released under the MIT License.
  http://opensource.org/licenses/mit-license.php
  */

#ifndef CLSPV_TEST_MAPPED_MEMORY_HPP
#define CLSPV_TEST_MAPPED_MEMORY_HPP

// Standard C++ library
#include <type_traits>
// ClspvTest
#include "config.hpp"

namespace clspvtest {

// Forward declaration
template <typename> class VulkanBuffer;

/*!
  */
template <typename T>
class MappedMemory
{
 public:
  using Type = std::remove_volatile_t<T>;
  using ConstType = std::add_const_t<Type>;
  using Reference = std::add_lvalue_reference_t<Type>;
  using ConstReference = std::add_lvalue_reference_t<ConstType>;
  using Pointer = std::add_pointer_t<Type>;
  using ConstPointer = std::add_pointer_t<ConstType>;
  using Iterator = Pointer;
  using ConstIterator = ConstPointer;
  using ConstBuffer = std::add_const_t<VulkanBuffer<std::remove_cv_t<T>>>;
  using ConstBufferP = std::add_pointer_t<ConstBuffer>;


  //! Create an empty memory
  MappedMemory() noexcept;

  //! Create a mapped memory
  MappedMemory(ConstBufferP buffer) noexcept;

  //! Move a memory data
  MappedMemory(MappedMemory&& other) noexcept;

  //! Destruct the managed memory
  ~MappedMemory() noexcept;


  // For STL
  //! Return an iterator to the first element of the container
  Iterator begin() noexcept;

  //! Return an iterator to the first element of the container
  ConstIterator begin() const noexcept;

  //! Return an iterator to the first element of the container
  ConstIterator cbegin() const noexcept;

  //! Return an iterator following the last element of the container
  Iterator end() noexcept;

  //! Return an iterator following the last element of the container
  ConstIterator end() const noexcept;

  //! Return an iterator following the last element of the container
  ConstIterator cend() const noexcept;


  //! Move a memory data
  MappedMemory& operator=(MappedMemory&& other) noexcept;

  //! Check whether this owns a memory
  explicit operator bool() const noexcept
  {
    return data() != nullptr;
  }

  //! Return the reference by index
  Reference operator[](const std::size_t index) noexcept;

  //! Return the reference by index
  ConstReference operator[](const std::size_t index) const noexcept;


  //! Return a pointer to the managed memory
  Pointer data() noexcept;

  //! Return a pointer to the managed memory
  ConstPointer data() const noexcept;

  //! Return a pointer to the managed memory
  Reference get(const std::size_t index) noexcept;

  //! Return a pointer to the managed memory
  ConstReference get(const std::size_t index) const noexcept;

  //! Set a value to the managed memory at index
  void set(const std::size_t index, ConstReference value) noexcept;

  //! Return a size of a memory array
  std::size_t size() const noexcept;

  //! Unmap the managed memory 
  void unmap() noexcept;

 private:
  static_assert(!std::is_pointer_v<Type>, "The Type is pointer.");
  static_assert(!std::is_reference_v<Type>, "The Type is reference.");


  Pointer data_ = nullptr;
  ConstBufferP buffer_ = nullptr;
};

} // namespace clspvtest

#include "mapped_memory-inl.hpp"

#endif // CLSPV_TEST_MAPPED_MEMORY_HPP
