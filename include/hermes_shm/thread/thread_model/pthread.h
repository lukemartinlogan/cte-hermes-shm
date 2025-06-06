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

#ifndef HSHM_THREAD_PTHREAD_H_
#define HSHM_THREAD_PTHREAD_H_

#include <errno.h>
#ifdef __APPLE__
#include <unistd.h>
#endif

#include "hermes_shm/introspect/system_info.h"
#include "hermes_shm/types/atomic.h"
#include "hermes_shm/util/errors.h"
#include "thread_model.h"

namespace hshm::thread {

class Pthread : public ThreadModel {
 public:
  ThreadLocalKey tid_key_;
  hipc::atomic<hshm::size_t> tid_counter_;

 public:
  /** Default constructor */
  HSHM_INLINE_CROSS_FUN
  Pthread() : ThreadModel(ThreadType::kPthread) {
    tid_counter_ = 1;
    CreateTls<void>(tid_key_, nullptr);
  }

  /** Destructor */
  ~Pthread() = default;

  /** Initialize pthread */
  HSHM_CROSS_FUN
  void Init() {}

  /** Yield the thread for a period of time */
  HSHM_CROSS_FUN
  void SleepForUs(size_t us) {
#ifdef HSHM_IS_HOST
    usleep(us);
#endif
  }

  /** Yield thread time slice */
  HSHM_CROSS_FUN
  void Yield() {
#ifdef HSHM_IS_HOST
    sched_yield();
#endif
  }

  /** Create thread-local storage */
  template <typename TLS>
  HSHM_CROSS_FUN bool CreateTls(ThreadLocalKey &key, TLS *data) {
#ifdef HSHM_IS_HOST
    int ret = pthread_key_create(&key.pthread_key_,
                                 ThreadLocalData::destroy_wrap<TLS>);
    if (ret != 0) {
      return false;
    }
    return SetTls(key, data);
#else
    return false;
#endif
  }

  /** Create thread-local storage */
  template <typename TLS>
  HSHM_CROSS_FUN bool SetTls(ThreadLocalKey &key, TLS *data) {
#ifdef HSHM_IS_HOST
    pthread_setspecific(key.pthread_key_, data);
    return true;
#else
    return false;
#endif
  }

  /** Get thread-local storage */
  template <typename TLS>
  HSHM_CROSS_FUN TLS *GetTls(const ThreadLocalKey &key) {
#ifdef HSHM_IS_HOST
    TLS *data = (TLS *)pthread_getspecific(key.pthread_key_);
    return data;
#else
    return nullptr;
#endif
  }

  /** Get the TID of the current thread */
  HSHM_CROSS_FUN
  ThreadId GetTid() {
#ifdef HSHM_IS_HOST
    size_t tid = (size_t)GetTls<void>(tid_key_);
    if (!tid) {
      tid = tid_counter_.fetch_add(1);
      SetTls<void>(tid_key_, (void *)tid);
    }
    tid -= 1;
    return ThreadId{(hshm::u64)tid};
#else
    return ThreadId{0};
#endif
  }

  /** Create a thread group */
  HSHM_CROSS_FUN
  ThreadGroup CreateThreadGroup(const ThreadGroupContext &ctx) {
    return ThreadGroup{};
  }

  /** Spawn a thread */
  template <typename FUNC, typename... Args>
  HSHM_CROSS_FUN Thread Spawn(ThreadGroup &group, FUNC &&func, Args &&...args) {
    Thread thread;
    thread.group_ = group;
    ThreadParams<FUNC, Args...> *params = new ThreadParams<FUNC, Args...>(
        std::forward<FUNC>(func), std::forward<Args>(args)...);
    pthread_create(&thread.pthread_thread_, nullptr,
                   SpawnWrapper<FUNC, Args...>, (void *)params);
    return thread;
  }

  /** Wrapper for spawning a thread */
  template <typename FUNC, typename... Args>
  static void *SpawnWrapper(void *arg) {
    ThreadParams<FUNC, Args...> *params =
        static_cast<ThreadParams<FUNC, Args...> *>(arg);
    PassArgPack::Call(std::forward<ArgPack<Args...>>(params->args_),
                      std::forward<FUNC>(params->func_));
    delete params;
    return nullptr;
  }

  /** Join a thread */
  HSHM_CROSS_FUN
  void Join(Thread &thread) {
#ifdef HSHM_IS_HOST
    pthread_join(thread.pthread_thread_, nullptr);
#endif
  }
};

}  // namespace hshm::thread

#endif  // HSHM_THREAD_PTHREAD_H_
