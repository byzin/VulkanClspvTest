/*!
  \file mapped_memory-inl.hpp
  \author Sho Ikeda
  */

#ifndef CLSPV_TEST_MAPPED_MEMORY_INL_HPP
#define CLSPV_TEST_MAPPED_MEMORY_INL_HPP

#include "mapped_memory.hpp"
// ClspvTest
#include "config.hpp"
#include "vulkan_buffer.hpp"

namespace clspvtest {

/*!
  */
template <typename T> inline
MappedMemory<T>::MappedMemory() noexcept
{
}

/*!
  */
template <typename T> inline
MappedMemory<T>::MappedMemory(ConstBufferP buffer) noexcept :
    data_{(buffer != nullptr) ? buffer->mappedMemory() : nullptr},
    buffer_{buffer}
{
}

/*!
  */
template <typename T> inline
MappedMemory<T>::MappedMemory(MappedMemory&& other) noexcept :
    data_{other.data_},
    buffer_{other.buffer_}
{
  other.data_ = nullptr;
  other.buffer_ = nullptr;
}

/*!
  */
template <typename T> inline
MappedMemory<T>::~MappedMemory() noexcept
{
  unmap();
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::begin() noexcept -> Iterator
{
  auto ite = data();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::begin() const noexcept -> ConstIterator
{
  auto ite = data();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::cbegin() const noexcept -> ConstIterator
{
  auto ite = data();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::end() noexcept -> Iterator
{
  auto ite = data();
  ite = ite + size();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::end() const noexcept -> ConstIterator
{
  auto ite = data();
  ite = ite + size();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::cend() const noexcept -> ConstIterator
{
  auto ite = data();
  ite = ite + size();
  return ite;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::operator=(MappedMemory&& other) noexcept -> MappedMemory&
{
  data_ = other.data_;
  buffer_ = other.buffer_;
  other.data_ = nullptr;
  other.buffer_ = nullptr;
  return *this;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::operator[](const std::size_t index) noexcept
    -> Reference
{
  return get(index);
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::operator[](const std::size_t index) const noexcept
    -> ConstReference
{
  return get(index);
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::data() noexcept -> Pointer
{
  return data_;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::data() const noexcept -> ConstPointer
{
  return data_;
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::get(const std::size_t index) noexcept
    -> Reference
{
  auto d = data();
  return d[index];
}

/*!
  */
template <typename T> inline
auto MappedMemory<T>::get(const std::size_t index) const noexcept
    -> ConstReference
{
  auto d = data();
  return d[index];
}

/*!
  */
template <typename T> inline
void MappedMemory<T>::set(const std::size_t index, ConstReference value) noexcept
{
  auto d = data();
  d[index] = value;
}

/*!
  */
template <typename T> inline
std::size_t MappedMemory<T>::size() const noexcept
{
  const std::size_t s = (buffer_ != nullptr) ? buffer_->size() : 0;
  return s;
}

/*!
  */
template <typename T> inline
void MappedMemory<T>::unmap() noexcept
{
  if (buffer_ != nullptr)
    buffer_->unmapMemory();
  data_ = nullptr;
  buffer_ = nullptr;
}

} // namespace clspvtest

#endif // CLSPV_TEST_MAPPED_MEMORY_INL_HPP
