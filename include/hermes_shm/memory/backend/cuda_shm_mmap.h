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

#ifndef HERMES_INCLUDE_MEMORY_BACKEND_CUDA_SHM_MMAP_H
#define HERMES_INCLUDE_MEMORY_BACKEND_CUDA_SHM_MMAP_H

#include "memory_backend.h"
#include "hermes_shm/util/logging.h"
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include <hermes_shm/util/errors.h>
#include <hermes_shm/constants/macros.h>
#include <hermes_shm/introspect/system_info.h>
#include <cuda_runtime.h>
#include "posix_shm_mmap.h"

namespace hshm::ipc {

class CudaShmMmap : public PosixShmMmap {
 public:
  /** Initialize shared memory */
  HSHM_CROSS_FUN
  bool shm_init(size_t size, const std::string &url, int device) {
#ifndef __CUDA_ARCH__
    cudaDeviceSynchronize();
    cudaSetDevice(device);
    return PosixShmMmap::shm_init(size, url);
#endif
  }

  /** Map shared memory */
  HSHM_CROSS_FUN
  char* _Map(size_t size, off64_t off) override {
#ifndef __CUDA_ARCH__
    char* ptr = _ShmMap(size, off);
    cudaHostRegister(ptr, size, cudaHostRegisterPortable);
    return ptr;
#endif
  }

  /** Detach shared memory */
  HSHM_CROSS_FUN
  void _Detach() override {
#ifndef __CUDA_ARCH__
    cudaHostUnregister(data_);
    _ShmDetach();
#endif
  }
};

}  // namespace hshm::ipc

#endif  // HERMES_INCLUDE_MEMORY_BACKEND_CUDA_SHM_MMAP_H
