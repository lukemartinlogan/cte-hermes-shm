/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of hermes
 *
 * hermes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef HERMES_DATA_STRUCTURES_SHM_AR_H_
#define HERMES_DATA_STRUCTURES_SHM_AR_H_

#include "hermes_shm/memory/memory.h"
#include "hermes_shm/data_structures/data_structure.h"
#include "hermes_shm/types/tuple_base.h"

namespace hermes::ipc {

/**
 * Constructs a TypedPointer in-place
 * */
template<typename T>
class _ShmHeaderOrT_Header {
 public:
  typedef typename T::header_t header_t;
  header_t obj_hdr_;

 public:
  /** Default constructor */
  _ShmHeaderOrT_Header() = default;

  /** Construct + store object */
  template<typename ...Args>
  explicit _ShmHeaderOrT_Header(Allocator *alloc, Args&& ...args) {
    T(obj_hdr_, alloc, std::forward<Args>(args)...).UnsetDestructable();
  }

  /** Construct + store object (hermes rval argpack) */
  template<typename ArgPackT>
  void PiecewiseInit(Allocator *alloc,
                     ArgPackT &&args) {
    T obj;
    PassArgPack::Call(
      MergeArgPacks::Merge(make_argpack(obj, obj_hdr_, alloc),
                           std::forward<ArgPackT>(args)),
      [](auto&& ...Args) constexpr {
        Allocator::ConstructObj<T>(std::forward<decltype(Args)>(Args)...);
      });
    obj.UnsetDestructable();
  }

  /** Destructor */
  ~_ShmHeaderOrT_Header() = default;

  /** Shm destructor */
  void shm_destroy(Allocator *alloc) {
    auto ar = internal_ref(alloc);
    T obj;
    obj.shm_deserialize(ar);
    obj.shm_destroy();
  }

  /** Returns a reference to the internal object */
  ShmDeserialize<T> internal_ref(Allocator *alloc) {
    return ShmDeserialize<T>(alloc, &obj_hdr_);
  }

  /** Returns a reference to the internal object */
  ShmDeserialize<T> internal_ref(Allocator *alloc) const {
    return ShmDeserialize<T>(alloc, &obj_hdr_);
  }

  /** Move constructor */
  _ShmHeaderOrT_Header(_ShmHeaderOrT_Header &&other) noexcept
  : obj_hdr_(std::move(other.obj_hdr_)) {}

  /** Move assignment operator */
  _ShmHeaderOrT_Header& operator=(_ShmHeaderOrT_Header &&other) noexcept {
    obj_hdr_ = std::move(other.obj_hdr_);
    return *this;
  }

  /** Copy constructor */
  _ShmHeaderOrT_Header(const _ShmHeaderOrT_Header &other)
  : obj_hdr_(other.obj_hdr_) {
  }

  /** Copy assignment operator */
  _ShmHeaderOrT_Header& operator=(const _ShmHeaderOrT_Header &other) {
    obj_hdr_ = other.obj_hdr_;
  }
};

/**
 * Constructs an object in-place
 * */
template<typename T>
class _ShmHeaderOrT_T {
 public:
  char obj_[sizeof(T)]; /**< Store object without constructing */

 public:
  /** Default constructor */
  _ShmHeaderOrT_T() {
    Allocator::ConstructObj<T>(internal_ref(nullptr));
  }

  /** Construct + store object (C++ argpack) */
  template<typename ...Args>
  explicit _ShmHeaderOrT_T(Allocator *alloc, Args&& ...args) {
    Allocator::ConstructObj<T>(
      internal_ref(alloc), std::forward<Args>(args)...);
  }

  /** Construct + store object (hermes rval argpack) */
  template<typename ArgPackT>
  void PiecewiseInit(Allocator *alloc,
                     ArgPackT &&args) {
    hermes::PassArgPack::Call(
      MergeArgPacks::Merge(
        make_argpack(internal_ref(alloc)),
        std::forward<ArgPackT>(args)),
      [](auto&& ...Args) constexpr {
        Allocator::ConstructObj<T>(std::forward<decltype(Args)>(Args)...);
      });
  }

  /** Shm destructor */
  void shm_destroy(Allocator *alloc) {}

  /** Destructor. Does nothing. */
   ~_ShmHeaderOrT_T() = default;

  /** Returns a reference to the internal object */
  T& internal_ref(Allocator *alloc) {
    (void) alloc;
    return reinterpret_cast<T&>(obj_);
  }

  /** Returns a reference to the internal object */
  T& internal_ref(Allocator *alloc) const {
    (void) alloc;
    return reinterpret_cast<T&>(obj_);
  }

  /** Move constructor */
  _ShmHeaderOrT_T(_ShmHeaderOrT_T &&other) noexcept {
    Allocator::ConstructObj<_ShmHeaderOrT_T>(
      obj_, std::move(other.internal_ref()));
  }

  /** Move assignment operator */
  _ShmHeaderOrT_T& operator=(_ShmHeaderOrT_T &&other) noexcept {
    internal_ref() = std::move(other.internal_ref());
    return *this;
  }

  /** Copy constructor */
  _ShmHeaderOrT_T(const _ShmHeaderOrT_T &other) {
    Allocator::ConstructObj<_ShmHeaderOrT_T>(
      obj_, other);
  }

  /** Copy assignment operator */
  _ShmHeaderOrT_T& operator=(const _ShmHeaderOrT_T &other) {
    internal_ref() = other.internal_ref();
    return *this;
  }
};

/**
 * Whether or not to use _ShmHeaderOrT or _ShmHeaderOrT_T
 * */
#define SHM_MAKE_HEADER_OR_T(T) \
  SHM_X_OR_Y(T, _ShmHeaderOrT_Header<T>, _ShmHeaderOrT_T<T>)

/**
 * Used for data structures which intend to store:
 * 1. An archive if the data type is SHM_ARCHIVEABLE
 * 2. The raw type if the data type is anything else
 *
 * E.g., used in unordered_map for storing collision entries.
 * E.g., used in a list for storing list entries.
 * */
template<typename T>
class ShmHeaderOrT {
 public:
  typedef SHM_ARCHIVE_OR_REF(T) T_Ar;
  typedef SHM_MAKE_HEADER_OR_T(T) T_Hdr;
  T_Hdr obj_;

  /** Default constructor */
  ShmHeaderOrT() = default;

  /** Construct + store object */
  template<typename ...Args>
  explicit ShmHeaderOrT(Args&& ...args)
  : obj_(std::forward<Args>(args)...) {}

  /** Construct + store object (hermes rval argpack) */
  template<typename ArgPackT>
  void PiecewiseInit(Allocator *alloc, ArgPackT &&args) {
    obj_.PiecewiseInit(alloc, std::forward<ArgPackT>(args));
  }

  /** Destructor */
  ~ShmHeaderOrT() = default;

  /** Returns a reference to the internal object */
  T_Ar internal_ref(Allocator *alloc) {
    return obj_.internal_ref(alloc);
  }

  /** Returns a reference to the internal object */
  T_Ar internal_ref(Allocator *alloc) const {
    return obj_.internal_ref(alloc);
  }

  /** Shm destructor */
  void shm_destroy(Allocator *alloc) {
    obj_.shm_destroy(alloc);
  }

  /** Move constructor */
  ShmHeaderOrT(ShmHeaderOrT &&other) noexcept
  : obj_(std::move(other.obj_)) {}

  /** Move assignment operator */
  ShmHeaderOrT& operator=(ShmHeaderOrT &&other) noexcept {
    obj_ = std::move(other.obj_);
    return *this;
  }

  /** Copy constructor */
  ShmHeaderOrT(const ShmHeaderOrT &other)
  : obj_(other.obj_) {}

  /** Copy assignment operator */
  ShmHeaderOrT& operator=(const ShmHeaderOrT &other) {
    obj_ = other.obj_;
  }
};

}  // namespace hermes::ipc

#endif  // HERMES_DATA_STRUCTURES_SHM_AR_H_
