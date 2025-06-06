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

#ifndef HSHM_MACROS_H
#define HSHM_MACROS_H

#include "hermes_shm/constants/settings.h"

/** For windows */
// #define _CRT_SECURE_NO_DEPRECATE

/** Function content selector for CUDA */
#ifdef __CUDA_ARCH__
#define HSHM_IS_CUDA_GPU
#endif

/** Function content selector for ROCm */
#if __HIP_DEVICE_COMPILE__
#define HSHM_IS_ROCM_GPU
#endif

/** Function content selector for CPU vs GPU */
#if defined(HSHM_IS_CUDA_GPU) || defined(HSHM_IS_ROCM_GPU)
#define HSHM_IS_GPU
#else
#define HSHM_IS_HOST
#endif

/** Import / export flags for MSVC DLLs */
#ifdef HSHM_COMPILER_MSVC
#define HSHM_DLL_EXPORT __declspec(dllexport)
#define HSHM_DLL_IMPORT __declspec(dllimport)
#else
#define HSHM_DLL_EXPORT __attribute__((visibility("default")))
#define HSHM_DLL_IMPORT __attribute__((visibility("default")))
#endif

// The following should be set internally in HSHM source files.
// This is used only internally by this project
// #define __HSHM_IS_COMPILING__

// The following should be set externally by your project.
// This is used to switch DLLs
// #define HSHM_COMPILING_DLL

/** DLL import / export for HSHM code */
#ifdef __HSHM_IS_COMPILING__
#define HSHM_DLL HSHM_DLL_EXPORT
#else
#define HSHM_DLL HSHM_DLL_IMPORT
#endif

/** DLL import / export for singletons */
#ifdef HSHM_COMPILING_DLL
#define HSHM_DLL_SINGLETON HSHM_DLL_EXPORT
#else
#define HSHM_DLL_SINGLETON HSHM_DLL_IMPORT
#endif

/**
 * Remove parenthesis surrounding "X" if it has parenthesis
 * Used for helper macros which take templated types as parameters
 * E.g., let's say we have:
 *
 * #define HELPER_MACRO(T) TYPE_UNWRAP(T)
 * HELPER_MACRO( (std::vector<std::pair<int, int>>) )
 * will return std::vector<std::pair<int, int>> without the parenthesis
 * */
#define TYPE_UNWRAP(X) ESC(ISH X)
#define ISH(...) ISH __VA_ARGS__
#define ESC(...) ESC_(__VA_ARGS__)
#define ESC_(...) VAN##__VA_ARGS__
#define VANISH
#define __TU(X) TYPE_UNWRAP(X)

#if defined(HSHM_ENABLE_CUDA) or defined(HSHM_ENABLE_ROCM)
#define HSHM_ENABLE_CUDA_OR_ROCM
#endif

/** Includes for CUDA and ROCm */
#ifdef HSHM_ENABLE_CUDA
#include <cuda_runtime.h>
#endif

#ifdef HSHM_ENABLE_ROCM
#include <hip/hip_runtime.h>
#endif

/** Macros for CUDA functions */
#if defined(HSHM_ENABLE_CUDA) || defined(HSHM_ENABLE_ROCM)
#define ROCM_HOST __host__
#define ROCM_DEVICE __device__
#define ROCM_HOST_DEVICE __device__ __host__
#define ROCM_KERNEL __global__
#else
#define ROCM_HOST_DEVICE
#define ROCM_HOST
#define ROCM_DEVICE
#define ROCM_KERNEL
#endif

/** Error checking for ROCM */
#define HIP_ERROR_CHECK(X)                                                  \
  do {                                                                      \
    if (X != hipSuccess) {                                                  \
      hipError_t hipErr = hipGetLastError();                                \
      HELOG(kFatal, "HIP Error {}: {}", hipErr, hipGetErrorString(hipErr)); \
    }                                                                       \
  } while (false)

/** Error checking for CUDA */
#define CUDA_ERROR_CHECK(X)                       \
  do {                                            \
    if (X != cudaSuccess) {                       \
      cudaError_t cudaErr = cudaGetLastError();   \
      HELOG(kFatal, "CUDA Error {}: {}", cudaErr, \
            cudaGetErrorString(cudaErr));         \
    }                                             \
  } while (false)

/**
 * Ensure that the compiler ALWAYS inlines a particular function.
 * */
#if defined(HSHM_COMPILER_MSVC)
#define HSHM_INLINE_FLAG __forceinline
#define HSHM_NO_INLINE_FLAG __declspec(noinline)
#define HSHM_FUNC_IS_USED __declspec(selectany)
#elif defined(HSHM_COMPILER_GNU)
#define HSHM_INLINE_FLAG __attribute__((always_inline))
#define HSHM_NO_INLINE_FLAG __attribute__((noinline))
#define HSHM_FUNC_IS_USED __attribute__((used))
#endif

#define HSHM_NO_INLINE HSHM_NO_INLINE_FLAG
#ifndef HSHM_DEBUG
#define HSHM_INLINE
#else
#define HSHM_INLINE inline HSHM_INLINE_FLAG
#endif

/** Macros for gpu/host function + var */
#define HSHM_HOST_FUN ROCM_HOST
#define HSHM_HOST_VAR ROCM_HOST
#define HSHM_GPU_FUN ROCM_DEVICE
#define HSHM_GPU_VAR ROCM_DEVICE
#define HSHM_CROSS_FUN ROCM_HOST_DEVICE
#define HSHM_GPU_KERNEL ROCM_KERNEL

/** Macro for inline gpu/host function + var */
#define HSHM_INLINE_CROSS_FUN HSHM_CROSS_FUN HSHM_INLINE
#define HSHM_INLINE_CROSS_VAR HSHM_CROSS_FUN inline
#define HSHM_INLINE_GPU_FUN ROCM_DEVICE HSHM_INLINE
#define HSHM_INLINE_GPU_VAR ROCM_DEVICE inline
#define HSHM_INLINE_HOST_FUN ROCM_HOST HSHM_INLINE
#define HSHM_INLINE_HOST_VAR ROCM_HOST inline

/** Macro for selective cross function */
#ifdef HSHM_IS_HOST
#define HSHM_CROSS_FUN_SEL HSHM_HOST_FUN
#define HSHM_INLINE_CROSS_FUN_SEL HSHM_INLINE_HOST_FUN
#else
#define HSHM_CROSS_FUN_SEL HSHM_GPU_FUN
#define HSHM_INLINE_CROSS_FUN_SEL HSHM_INLINE_GPU_FUN
#endif

/** Test cross functions */
#define HSHM_NO_INLINE_CROSS_FUN HSHM_NO_INLINE HSHM_CROSS_FUN HSHM_FUNC_IS_USED

/** Bitfield macros */
#define MARK_FIRST_BIT_MASK(T) ((T)1 << (sizeof(T) * 8 - 1))
#define MARK_FIRST_BIT(T, X) ((X) | MARK_FIRST_BIT_MASK(T))
#define IS_FIRST_BIT_MARKED(T, X) ((X) & MARK_FIRST_BIT_MASK(T))
#define UNMARK_FIRST_BIT(T, X) ((X) & ~MARK_FIRST_BIT_MASK(T))

/** Class constant macro */
#define CLS_CONST static inline constexpr const
#define CLS_CROSS_CONST CLS_CONST

/** Class constant macro */
#ifdef HSHM_IS_HOST
#define GLOBAL_CONST inline const
#define GLOBAL_CROSS_CONST inline const
#else
#define GLOBAL_CONST inline const
#define GLOBAL_CROSS_CONST inline const __device__ __constant__
#endif

/** Namespace definitions */
namespace hshm {}
namespace hshm::ipc {}
namespace hipc = hshm::ipc;

/** The name of the current device */
#define HSHM_DEV_TYPE_CPU 0
#define HSHM_DEV_TYPE_GPU 1
#ifdef HSHM_IS_HOST
#define kCurrentDevice "cpu"
#define kCurrentDeviceType HSHM_DEV_TYPE_CPU
#define HSHM_GPU_OR_HOST host
#else
#define kCurrentDevice "gpu"
#define kCurrentDeviceType HSHM_DEV_TYPE_GPU
#define HSHM_GPU_OR_HOST gpu
#endif

/***************************************************
 * CUSTOM SETTINGS FOR ALLOCATORS + THREAD MODELS
 * ************************************************* */
/** Define the root allocator class */
#ifndef HSHM_ROOT_ALLOC_T
#define HSHM_ROOT_ALLOC_T hipc::StackAllocator
#endif
#define HSHM_ROOT_ALLOC \
  HSHM_MEMORY_MANAGER->template GetRootAllocator<HSHM_ROOT_ALLOC_T>()

/** Define the default allocator class */
#ifndef HSHM_DEFAULT_ALLOC_T
// #define HSHM_DEFAULT_ALLOC_T hipc::MallocAllocator
#define HSHM_DEFAULT_ALLOC_T hipc::ThreadLocalAllocator
#endif
#define HSHM_DEFAULT_ALLOC \
  HSHM_MEMORY_MANAGER->template GetDefaultAllocator<HSHM_DEFAULT_ALLOC_T>()

/** Define the default thread model class */
// CUDA
#if defined(HSHM_IS_CUDA_GPU) && !defined(HSHM_DEFAULT_THREAD_MODEL_GPU)
#define HSHM_DEFAULT_THREAD_MODEL_GPU hshm::thread::Cuda
#endif
// ROCM
#if defined(HSHM_IS_ROCM_GPU) && !defined(HSHM_DEFAULT_THREAD_MODEL_GPU)
#define HSHM_DEFAULT_THREAD_MODEL_GPU hshm::thread::Rocm
#endif
// CPU
#ifndef HSHM_DEFAULT_THREAD_MODEL
#if defined(HSHM_ENABLE_PTHREADS)
#define HSHM_DEFAULT_THREAD_MODEL hshm::thread::Pthread
#elif defined(HSHM_ENABLE_WINDOWS_THREADS)
#define HSHM_DEFAULT_THREAD_MODEL hshm::thread::StdThread
#endif
#endif

/** Default memory context object */
#define HSHM_DEFAULT_MEM_CTX (hipc::MemContext{})
#define HSHM_MCTX HSHM_DEFAULT_MEM_CTX

/** Compatability hack for static_assert */
template <bool TRUTH, typename T = int>
class assert_hack {
 public:
  CLS_CONST bool value = TRUTH;
};

/** A hack for static asserts */
#define STATIC_ASSERT(TRUTH, MSG, T) \
  static_assert(assert_hack<TRUTH, __TU(T)>::value, MSG)

#endif  // HSHM_MACROS_H
