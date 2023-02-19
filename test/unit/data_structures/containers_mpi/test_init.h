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


#ifndef HERMES_TEST_UNIT_DATA_STRUCTURES_TEST_INIT_H_
#define HERMES_TEST_UNIT_DATA_STRUCTURES_TEST_INIT_H_

#include "hermes_shm/memory/allocator/page_allocator.h"
#include "hermes_shm/data_structures/data_structure.h"
#include <mpi.h>

using hermes::ipc::PosixShmMmap;
using hermes::ipc::MemoryBackendType;
using hermes::ipc::MemoryBackend;
using hermes::ipc::allocator_id_t;
using hermes::ipc::AllocatorType;
using hermes::ipc::Allocator;
using hermes::ipc::Pointer;

using hermes::ipc::MemoryBackendType;
using hermes::ipc::MemoryBackend;
using hermes::ipc::allocator_id_t;
using hermes::ipc::AllocatorType;
using hermes::ipc::Allocator;
using hermes::ipc::MemoryManager;
using hermes::ipc::Pointer;

extern Allocator *alloc_g;

void Posttest();

#endif  // HERMES_TEST_UNIT_DATA_STRUCTURES_TEST_INIT_H_
