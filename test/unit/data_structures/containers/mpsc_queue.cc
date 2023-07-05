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

#include "basic_test.h"
#include "test_init.h"
#include "hermes_shm/data_structures/ipc/mpsc_queue.h"
#include "hermes_shm/data_structures/ipc/mpsc_ptr_queue.h"
#include "queue.h"

/**
 * TEST MPSC QUEUE
 * */

TEST_CASE("TestMpscQueueInt") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceThenConsume<hipc::mpsc_queue<int>, int>(1, 1, 32, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

TEST_CASE("TestMpscQueueString") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceThenConsume<hipc::mpsc_queue<hipc::string>, hipc::string>(
    1, 1, 32, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

TEST_CASE("TestMpscQueueIntMultiThreaded") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceAndConsume<hipc::mpsc_queue<int>, int>(8, 1, 8192, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

TEST_CASE("TestMpscQueueStringMultiThreaded") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceAndConsume<hipc::mpsc_queue<hipc::string>, hipc::string>(
    8, 1, 8192, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

/**
 * MPSC Pointer Queue
 * */

TEST_CASE("TestMpscPtrQueueInt") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceThenConsume<hipc::mpsc_ptr_queue<int>, int>(1, 1, 32, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

TEST_CASE("TestMpscPtrQueueIntMultiThreaded") {
  Allocator *alloc = alloc_g;
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
  ProduceAndConsume<hipc::mpsc_ptr_queue<int>, int>(8, 1, 8192, 32);
  REQUIRE(alloc->GetCurrentlyAllocatedSize() == 0);
}

TEST_CASE("TestMpscOffsetPointerQueueCompile") {
  Allocator *alloc = alloc_g;
  auto p = hipc::make_uptr<hipc::mpsc_ptr_queue<hipc::OffsetPointer>>(alloc);
  hipc::OffsetPointer off_p;
  p->emplace(hipc::OffsetPointer(5));
  p->pop(off_p);
  REQUIRE(off_p == hipc::OffsetPointer(5));
}

TEST_CASE("TestMpscPointerQueueCompile") {
  Allocator *alloc = alloc_g;
  auto p = hipc::make_uptr<hipc::mpsc_ptr_queue<hipc::Pointer>>(alloc);
  hipc::Pointer off_p;
  p->emplace(hipc::Pointer(allocator_id_t(5, 2), 1));
  p->pop(off_p);
  REQUIRE(off_p == hipc::Pointer(allocator_id_t(5, 2), 1));
}
