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


#ifndef HERMES_DATA_STRUCTURES_DATA_STRUCTURE_H_
#define HERMES_DATA_STRUCTURES_DATA_STRUCTURE_H_

#include "internal/shm_internal.h"
#include "hermes_shm/memory/memory_manager.h"

#include "ipc/functional.h"
#include "ipc/tuple_base.h"

#include "ipc/chararr.h"
#include "ipc/iqueue.h"
#include "ipc/list.h"
#include "ipc/pair.h"
#include "ipc/pod_array.h"
#include "ipc/ring_ptr_queue.h"
#include "ipc/ring_queue.h"
#include "ipc/slist.h"
#include "ipc/split_ticket_queue.h"
#include "ipc/string.h"
#include "ipc/ticket_queue.h"
#include "ipc/unordered_map.h"
#include "ipc/vector.h"
#include "ipc/key_set.h"
#include "ipc/key_queue.h"

#include "serialization/serialize_common.h"
#include "serialization/local_serialize.h"

#define HSHM_DATA_STRUCTURES_TEMPLATE_BASE(NS, HSHM_NS, ALLOC_T)            \
  namespace NS {                                                            \
  template <int LENGTH, bool WithNull>                                      \
  using chararr_templ = HSHM_NS::chararr_templ<LENGTH, WithNull>;           \
                                                                            \
  using HSHM_NS::chararr;                                                   \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using iqueue = HSHM_NS::iqueue<T, AllocT>;                                \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using list = HSHM_NS::list<T, AllocT>;                                    \
                                                                            \
  template <typename FirstT, typename SecondT, typename AllocT = ALLOC_T>   \
  using pair = HSHM_NS::pair<FirstT, SecondT, AllocT>;                      \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using spsc_queue = HSHM_NS::spsc_queue<T, AllocT>;                        \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using mpsc_queue = HSHM_NS::mpsc_queue<T, AllocT>;                        \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_spsc_queue = HSHM_NS::fixed_spsc_queue<T, AllocT>;            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_mpsc_queue = HSHM_NS::fixed_mpsc_queue<T, AllocT>;            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_mpmc_queue = HSHM_NS::fixed_mpmc_queue<T, AllocT>;            \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using spsc_ptr_queue = HSHM_NS::spsc_ptr_queue<T, AllocT>;                \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using mpsc_ptr_queue = HSHM_NS::mpsc_ptr_queue<T, AllocT>;                \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_spsc_ptr_queue = HSHM_NS::fixed_spsc_ptr_queue<T, AllocT>;    \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_mpsc_ptr_queue = HSHM_NS::fixed_mpsc_ptr_queue<T, AllocT>;    \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using fixed_mpmc_ptr_queue = HSHM_NS::fixed_mpmc_ptr_queue<T, AllocT>;    \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using slist = HSHM_NS::slist<T, AllocT>;                                  \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using split_ticket_queue = HSHM_NS::split_ticket_queue<T, AllocT>;        \
                                                                            \
  template <typename AllocT = ALLOC_T>                                      \
  using string = HSHM_NS::string_templ<HSHM_STRING_SSO, 0, AllocT>;         \
                                                                            \
  template <typename AllocT = ALLOC_T>                                      \
  using charbuf = HSHM_NS::string_templ<HSHM_STRING_SSO, 0, AllocT>;        \
                                                                            \
  template <typename AllocT = ALLOC_T>                                      \
  using charwrap = HSHM_NS::string_templ<HSHM_STRING_SSO,                   \
                                         hipc::StringFlags::kWrap, AllocT>; \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using ticket_queue = HSHM_NS::ticket_queue<T, AllocT>;                    \
                                                                            \
  template <typename Key, typename T, class Hash = hshm::hash<Key>,         \
            typename AllocT = ALLOC_T>                                      \
  using unordered_map = HSHM_NS::unordered_map<Key, T, Hash, AllocT>;       \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using vector = HSHM_NS::vector<T, AllocT>;                                \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using key_set = HSHM_NS::key_set<T, AllocT>;                              \
                                                                            \
  template <typename T, typename AllocT = ALLOC_T>                          \
  using key_queue = HSHM_NS::key_queue<T, AllocT>;                          \
  }

#define HSHM_DATA_STRUCTURES_TEMPLATE(NS, ALLOC_T)      \
  HSHM_DATA_STRUCTURES_TEMPLATE_BASE(NS, hshm, ALLOC_T) \
  HSHM_DATA_STRUCTURES_TEMPLATE_BASE(NS::ipc, hipc, ALLOC_T)

#endif  // HERMES_DATA_STRUCTURES_DATA_STRUCTURE_H_
