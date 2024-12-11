/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#ifndef HERMES_MEMORY_MEMORY_H_
#define HERMES_MEMORY_MEMORY_H_

#include <hermes_shm/types/real_number.h>
#include <hermes_shm/introspect/system_info.h>
#include <hermes_shm/types/bitfield.h>
#include <hermes_shm/types/atomic.h>
#include <hermes_shm/constants/macros.h>
#include <hermes_shm/data_structures/ipc/hash.h>
#include <cstdio>

namespace hshm::ipc {

/**
 * The identifier for an allocator
 * */
union AllocatorId {
  struct {
    uint32_t major_;  // Typically some sort of process id
    uint32_t minor_;  // Typically a process-local id
  } bits_;
  uint64_t int_;

  HSHM_INLINE_CROSS AllocatorId() = default;

  /**
   * Constructor which sets major & minor
   * */
  HSHM_INLINE_CROSS explicit AllocatorId(uint32_t major, uint32_t minor) {
    bits_.major_ = major;
    bits_.minor_ = minor;
  }

  /**
   * Set this allocator to null
   * */
  HSHM_INLINE_CROSS void SetNull() {
    int_ = 0;
  }

  /**
   * Check if this is the null allocator
   * */
  HSHM_INLINE_CROSS bool IsNull() const { return int_ == 0; }

  /** Equality check */
  HSHM_INLINE_CROSS bool operator==(const AllocatorId &other) const {
    return other.int_ == int_;
  }

  /** Inequality check */
  HSHM_INLINE_CROSS bool operator!=(const AllocatorId &other) const {
    return other.int_ != int_;
  }

  /** Get the null allocator */
  HSHM_INLINE_CROSS static AllocatorId GetNull() {
    return AllocatorId(0, 0);
  }

  /** To index */
  HSHM_INLINE_CROSS uint32_t ToIndex() const {
    return bits_.major_ * 4 + bits_.minor_;
  }

  /** Serialize an hipc::allocator_id */
  template <typename Ar>
  HSHM_INLINE_CROSS
  void serialize(Ar &ar) {
    ar &int_;
  }

  /** Print */
  HSHM_CROSS_FUN
  void Print() const {
    printf("(%s) Allocator ID: %u.%u\n",
           kCurrentDevice, bits_.major_, bits_.minor_);
  }
};

typedef AllocatorId allocator_id_t;

/**
 * Stores an offset into a memory region. Assumes the developer knows
 * which allocator the pointer comes from.
 * */
template<bool ATOMIC = false>
struct OffsetPointerBase {
  hipc::opt_atomic<size_t, ATOMIC> off_; /**< Offset within the allocator's slot */

  /** Default constructor */
  HSHM_INLINE_CROSS OffsetPointerBase() = default;

  /** Full constructor */
  HSHM_INLINE_CROSS explicit OffsetPointerBase(size_t off) : off_(off) {}

  /** Full constructor */
  HSHM_INLINE_CROSS explicit OffsetPointerBase(
      hipc::opt_atomic<size_t, ATOMIC> off)
  : off_(off.load()) {}

  /** Pointer constructor */
  HSHM_INLINE_CROSS explicit OffsetPointerBase(AllocatorId alloc_id,
                                                size_t off)
  : off_(off) {
    (void) alloc_id;
  }

  /** Copy constructor */
  HSHM_INLINE_CROSS OffsetPointerBase(const OffsetPointerBase &other)
  : off_(other.off_.load()) {}

  /** Other copy constructor */
  HSHM_INLINE_CROSS OffsetPointerBase(const OffsetPointerBase<!ATOMIC> &other)
  : off_(other.off_.load()) {}

  /** Move constructor */
  HSHM_INLINE_CROSS OffsetPointerBase(OffsetPointerBase &&other) noexcept
    : off_(other.off_.load()) {
    other.SetNull();
  }

  /** Get the offset pointer */
  HSHM_INLINE_CROSS OffsetPointerBase<false> ToOffsetPointer() {
    return OffsetPointerBase<false>(off_.load());
  }

  /** Set to null */
  HSHM_INLINE_CROSS void SetNull() {
    off_ = (size_t)-1;
  }

  /** Check if null */
  HSHM_INLINE_CROSS bool IsNull() const {
    return off_.load() == (size_t)-1;
  }

  /** Get the null pointer */
  HSHM_INLINE_CROSS static OffsetPointerBase GetNull() {
    return OffsetPointerBase((size_t)-1);
  }

  /** Atomic load wrapper */
  HSHM_INLINE_CROSS size_t load(
    std::memory_order order = std::memory_order_seq_cst) const {
    return off_.load(order);
  }

  /** Atomic exchange wrapper */
  HSHM_INLINE_CROSS void exchange(
    size_t count, std::memory_order order = std::memory_order_seq_cst) {
    off_.exchange(count, order);
  }

  /** Atomic compare exchange weak wrapper */
  HSHM_INLINE_CROSS bool compare_exchange_weak(
    size_t& expected, size_t desired,
    std::memory_order order = std::memory_order_seq_cst) {
    return off_.compare_exchange_weak(expected, desired, order);
  }

  /** Atomic compare exchange strong wrapper */
  HSHM_INLINE_CROSS bool compare_exchange_strong(
    size_t& expected, size_t desired,
    std::memory_order order = std::memory_order_seq_cst) {
    return off_.compare_exchange_weak(expected, desired, order);
  }

  /** Atomic add operator */
  HSHM_INLINE_CROSS OffsetPointerBase operator+(size_t count) const {
    return OffsetPointerBase(off_ + count);
  }

  /** Atomic subtract operator */
  HSHM_INLINE_CROSS OffsetPointerBase operator-(size_t count) const {
    return OffsetPointerBase(off_ - count);
  }

  /** Atomic add assign operator */
  HSHM_INLINE_CROSS OffsetPointerBase& operator+=(size_t count) {
    off_ += count;
    return *this;
  }

  /** Atomic subtract assign operator */
  HSHM_INLINE_CROSS OffsetPointerBase& operator-=(size_t count) {
    off_ -= count;
    return *this;
  }

  /** Atomic assign operator */
  HSHM_INLINE_CROSS OffsetPointerBase& operator=(size_t count) {
    off_ = count;
    return *this;
  }

  /** Atomic copy assign operator */
  HSHM_INLINE_CROSS OffsetPointerBase& operator=(
    const OffsetPointerBase &count) {
    off_ = count.load();
    return *this;
  }

  /** Equality check */
  HSHM_INLINE_CROSS bool operator==(const OffsetPointerBase &other) const {
    return off_ == other.off_;
  }

  /** Inequality check */
  HSHM_INLINE_CROSS bool operator!=(const OffsetPointerBase &other) const {
    return off_ != other.off_;
  }
};

/** Non-atomic offset */
typedef OffsetPointerBase<false> OffsetPointer;

/** Atomic offset */
typedef OffsetPointerBase<true> AtomicOffsetPointer;

/** Typed offset pointer */
template<typename T>
using TypedOffsetPointer = OffsetPointer;

/** Typed atomic pointer */
template<typename T>
using TypedAtomicOffsetPointer = AtomicOffsetPointer;

/**
 * A process-independent pointer, which stores both the allocator's
 * information and the offset within the allocator's region
 * */
template<bool ATOMIC = false>
struct PointerBase {
  AllocatorId allocator_id_;     /// Allocator the pointer comes from
  OffsetPointerBase<ATOMIC> off_;   /// Offset within the allocator's slot

  /** Default constructor */
  HSHM_INLINE_CROSS PointerBase() = default;

  /** Full constructor */
  HSHM_INLINE_CROSS explicit PointerBase(AllocatorId id, size_t off)
  : allocator_id_(id), off_(off) {}

  /** Full constructor using offset pointer */
  HSHM_INLINE_CROSS explicit PointerBase(AllocatorId id, OffsetPointer off)
  : allocator_id_(id), off_(off) {}

  /** Copy constructor */
  HSHM_INLINE_CROSS PointerBase(const PointerBase &other)
  : allocator_id_(other.allocator_id_), off_(other.off_) {}

  /** Other copy constructor */
  HSHM_INLINE_CROSS PointerBase(const PointerBase<!ATOMIC> &other)
  : allocator_id_(other.allocator_id_), off_(other.off_.load()) {}

  /** Move constructor */
  HSHM_INLINE_CROSS PointerBase(PointerBase &&other) noexcept
  : allocator_id_(other.allocator_id_), off_(other.off_) {
    other.SetNull();
  }

  /** Get the offset pointer */
  HSHM_INLINE_CROSS OffsetPointerBase<false> ToOffsetPointer() const {
    return OffsetPointerBase<false>(off_.load());
  }

  /** Set to null */
  HSHM_INLINE_CROSS void SetNull() {
    allocator_id_.SetNull();
  }

  /** Check if null */
  HSHM_INLINE_CROSS bool IsNull() const {
    return allocator_id_.IsNull();
  }

  /** Get the null pointer */
  HSHM_INLINE_CROSS static PointerBase GetNull() {
    static const PointerBase p(AllocatorId::GetNull(),
                               OffsetPointer::GetNull());
    return p;
  }

  /** Copy assignment operator */
  HSHM_INLINE_CROSS PointerBase& operator=(const PointerBase &other) {
    if (this != &other) {
      allocator_id_ = other.allocator_id_;
      off_ = other.off_;
    }
    return *this;
  }

  /** Move assignment operator */
  HSHM_INLINE_CROSS PointerBase& operator=(PointerBase &&other) {
    if (this != &other) {
      allocator_id_ = other.allocator_id_;
      off_.exchange(other.off_.load());
      other.SetNull();
    }
    return *this;
  }

  /** Addition operator */
  HSHM_INLINE_CROSS PointerBase operator+(size_t size) const {
    PointerBase p;
    p.allocator_id_ = allocator_id_;
    p.off_ = off_ + size;
    return p;
  }

  /** Subtraction operator */
  HSHM_INLINE_CROSS PointerBase operator-(size_t size) const {
    PointerBase p;
    p.allocator_id_ = allocator_id_;
    p.off_ = off_ - size;
    return p;
  }

  /** Addition assignment operator */
  HSHM_INLINE_CROSS PointerBase& operator+=(size_t size) {
    off_ += size;
    return *this;
  }

  /** Subtraction assignment operator */
  HSHM_INLINE_CROSS PointerBase& operator-=(size_t size) {
    off_ -= size;
    return *this;
  }

  /** Equality check */
  HSHM_INLINE_CROSS bool operator==(const PointerBase &other) const {
    return (other.allocator_id_ == allocator_id_ && other.off_ == off_);
  }

  /** Inequality check */
  HSHM_INLINE_CROSS bool operator!=(const PointerBase &other) const {
    return (other.allocator_id_ != allocator_id_ || other.off_ != off_);
  }
};

/** Non-atomic pointer */
typedef PointerBase<false> Pointer;

/** Atomic pointer */
typedef PointerBase<true> AtomicPointer;

/** Typed pointer */
template<typename T>
using TypedPointer = Pointer;

/** Typed atomic pointer */
template<typename T>
using TypedAtomicPointer = AtomicPointer;

/** Struct containing both private and shared pointer */
template<typename T = char, typename PointerT = Pointer>
struct LPointer {
  T *ptr_;
  PointerT shm_;

  /** Overload arrow */
  HSHM_INLINE_CROSS T* operator->() const {
    return ptr_;
  }

  /** Overload dereference */
  HSHM_INLINE_CROSS T& operator*() const {
    return *ptr_;
  }

  /** Check if null */
  HSHM_INLINE_CROSS bool IsNull() const {
    return ptr_ == nullptr;
  }

  /** Set to null */
  HSHM_INLINE_CROSS void SetNull() { ptr_ = nullptr; }

  /** Reintrepret cast to other internal type */
  template <typename U>
  HSHM_INLINE_CROSS
  LPointer<U, PointerT>& Cast() {
    return DeepCast<LPointer<U, PointerT>>();
  }

  /** Reintrepret cast to other internal type (const) */
  template <typename U>
  HSHM_INLINE_CROSS const LPointer<U, PointerT> &Cast() const {
    return DeepCast<LPointer<U, PointerT>>();
  }

  /** Reintrepret cast to another LPointer */
  template <typename LPointerT>
  HSHM_INLINE_CROSS
  LPointerT& DeepCast() {
    return *((LPointerT*)this);
  }

  /** Reintrepret cast to another LPointer (const) */
  template <typename LPointerT>
  HSHM_INLINE_CROSS
  const LPointerT& DeepCast() const {
    return *((LPointerT*)this);
  }
};

/** Alias to local pointer */
template<typename T = char, typename PointerT = Pointer>
using FullPtr = LPointer<T, PointerT>;

/** Struct containing both a pointer and its size */
template<typename PointerT = Pointer>
struct Array {
  PointerT shm_;
  size_t size_;
};

/** Struct containing a shared pointer, private pointer, and the data size */
template<typename T = char, typename PointerT = Pointer>
struct LArray {
  PointerT shm_;
  size_t size_;
  T *ptr_;

  /** Overload arrow */
  HSHM_INLINE_CROSS T* operator->() const {
    return ptr_;
  }

  /** Overload dereference */
  HSHM_INLINE_CROSS T& operator*() const {
    return *ptr_;
  }
};

class MemoryAlignment {
 public:
  /**
   * Round up to the nearest multiple of the alignment
   * @param alignment the alignment value (e.g., 4096)
   * @param size the size to make a multiple of alignment (e.g., 4097)
   * @return the new size  (e.g., 8192)
   * */
  static size_t AlignTo(size_t alignment,
                        size_t size) {
    auto page_size = HERMES_SYSTEM_INFO->page_size_;
    size_t new_size = size;
    size_t page_off = size % alignment;
    if (page_off) {
      new_size = size + page_size - page_off;
    }
    return new_size;
  }

  /**
   * Round up to the nearest multiple of page size
   * @param size the size to align to the PAGE_SIZE
   * */
  static size_t AlignToPageSize(size_t size) {
    auto page_size = HERMES_SYSTEM_INFO->page_size_;
    size_t new_size = AlignTo(page_size, size);
    return new_size;
  }
};

}  // namespace hshm::ipc

namespace std {

/** Allocator ID hash */
template <>
struct hash<hshm::ipc::AllocatorId> {
  HSHM_INLINE_CROSS std::size_t operator()(
    const hshm::ipc::AllocatorId &key) const {
    return hshm::hash<uint64_t>{}(key.int_);
  }
};

}  // namespace std

#define IS_SHM_OFFSET_POINTER(T) \
  std::is_same_v<T, OffsetPointer> || \
  std::is_same_v<T, AtomicOffsetPointer>

#define IS_SHM_POINTER(T) \
  std::is_same_v<T, Pointer> || \
  std::is_same_v<T, AtomicPointer>

#endif  // HERMES_MEMORY_MEMORY_H_
