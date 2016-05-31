/**
 * Copyright 2015 Facebook
 * @author Tudor Bosman (tudorb@fb.com)
 */

#ifndef THPP_CUDA_STORAGE_H_
#define THPP_CUDA_STORAGE_H_

#include <thpp/Storage.h>
#include <thpp/cuda/detail/Storage.h>
#include <folly/Malloc.h>
#include <folly/Range.h>

namespace thpp {

template <class T> class CudaTensor;

template <class T>
class CudaStorage : public StorageBase<T, CudaStorage<T>> {
  typedef StorageBase<T, CudaStorage<T>> Base;
  typedef typename Base::Ops Ops;
  friend Base;  // Yay C++11
 public:
  typedef typename Base::THType THType;
  CudaStorage();

  explicit CudaStorage(THType* t);

  explicit CudaStorage(const Storage<T>& cpuStorage);

  // Deserialize from Thrift. Throws if wrong type.
  explicit CudaStorage(const ThriftStorage& thriftStorage,
                       SharingMode sharing = SHARE_IOBUF_MANAGED);

  // Note that, despite being default (for consistency with the Storage
  // constructors), resizable == true is not yet implemented.
  explicit CudaStorage(folly::IOBuf&& iob,
                       SharingMode sharing = SHARE_IOBUF_MANAGED,
                       bool resizable = true);
  explicit CudaStorage(const folly::IOBuf& iob,
                       SharingMode sharing = SHARE_IOBUF_MANAGED,
                       bool resizable = true)
    : CudaStorage(folly::IOBuf(iob), sharing, resizable) { }

  ~CudaStorage();

  CudaStorage(CudaStorage&& other) noexcept;
  CudaStorage(const CudaStorage& other);
  CudaStorage& operator=(CudaStorage&& other);
  CudaStorage& operator=(const CudaStorage& other);

  // Serialize to Thrift.
  void serialize(ThriftStorage& out,
                 ThriftTensorEndianness endianness =
                     ThriftTensorEndianness::NATIVE,
                 bool mayShare = true) const;

  Storage<T> toCPU() const;

  T read(size_t offset) const;
  void read(size_t offset, T* dest, size_t n) const;
  void write(size_t offset, T value);
  void write(size_t offset, const T* src, size_t n);

  bool isUnique() const { return isUnique(this->t_); }
  // No CUDA support for custom allocators.
  static bool isUnique(const THType* th) {
    return !th || th->refcount == 1;
  }

  void setFromIOBuf(folly::IOBuf&& iob, SharingMode sharing, bool resizable);

 private:
  template <class U> friend class CudaTensor;
};

/**
 * Wrap a THCAllocator-like object with a C++ interface into THCAllocator.
 */
template <class A>
class THCAllocatorWrapper {
 public:
  static THCAllocator thcAllocator;
 private:
  static cudaError_t malloc(THCState* state, void* ctx, void** ptr,
                            long size) {
    return static_cast<A*>(ctx)->malloc(state, ptr, size);
  }
  static cudaError_t realloc(THCState* state, void* ctx, void** ptr,
                             long oldSize, long newSize) {
    return static_cast<A*>(ctx)->realloc(state, ptr, oldSize, newSize);
  }
  static cudaError_t free(THCState* state, void* ctx, void* ptr) {
    return static_cast<A*>(ctx)->free(state, ptr);
  }
};

}  // namespaces

#include <thpp/cuda/Storage-inl.h>

#endif /* THPP_CUDA_STORAGE_H_ */
