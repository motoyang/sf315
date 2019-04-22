#pragma once

#include <functional>

#include "types.hpp"
#include "utilities.hpp"

namespace uvp {

class Thread {
  mutable uv::ThreadT _thread;

public:
  using Callback = std::function<void(void *)>;

protected:
  struct Impl {
    void *arg;
    Callback _callback;
  };
  Impl _impl;

  static void callback(void *arg) {
    auto p = (Thread *)arg;
    if (p->_impl._callback) {
      p->_impl._callback(p->_impl.arg);
    } else {
      UVP_ASSERT(false);
    }
  }

public:
  int create(const Callback &cb, void *arg) {
    UVP_ASSERT(cb);
    _impl._callback = cb;
    _impl.arg = arg;

    int r = uv_thread_create(&_thread, Thread::callback, this);
    UVP_LOG_ERROR(r);
    return r;
  }

  int join() {
    int r = uv_thread_join(&_thread);
    UVP_LOG_ERROR(r);
    return r;
  }

  int equal(const Thread *t2) const {
    int r = uv_thread_equal(&_thread, &t2->_thread);
    UVP_LOG_ERROR(r);
    return r;
  }
};
// --

class Key {
  mutable uv::KeyT _key;

public:
  Key() {
    int r = uv_key_create(&_key);
    UVP_LOG_ERROR_EXIT(r);
  }
  virtual ~Key() { uv_key_delete(&_key); }

  void *get() const { return uv_key_get(&_key); }

  void set(void *value) { return uv_key_set(&_key, value); }
};

// --

class Mutex {
  friend class Cond;
  uv::MutexT _mutex;

public:
  Mutex(bool recursive = false) {
    int r = 0;
    if (recursive) {
      uv_mutex_init_recursive(&_mutex);
    } else {
      uv_mutex_init(&_mutex);
    }
    UVP_LOG_ERROR_EXIT(r);
  }
  virtual ~Mutex() { uv_mutex_destroy(&_mutex); }

  void lock() { return uv_mutex_lock(&_mutex); }

  int tryLock() {
    int r = uv_mutex_trylock(&_mutex);
    UVP_LOG_ERROR(r);
    return r;
  }

  void unlock() { return uv_mutex_unlock(&_mutex); }
};

class Rwlock {
  uv::Rwlock _rwlock;

public:
  Rwlock() {
    int r = uv_rwlock_init(&_rwlock);
    UVP_LOG_ERROR_EXIT(r);
  }
  virtual ~Rwlock() { uv_rwlock_destroy(&_rwlock); }

  void rdlock() { return uv_rwlock_rdlock(&_rwlock); }

  int tryRdlock() {
    int r = uv_rwlock_tryrdlock(&_rwlock);
    UVP_LOG_ERROR(r);
    return r;
  }

  void rdunlock() { return uv_rwlock_rdunlock(&_rwlock); }

  void wrlock() { return uv_rwlock_wrlock(&_rwlock); }

  int tryWrlock() {
    int r = uv_rwlock_trywrlock(&_rwlock);
    UVP_LOG_ERROR(r);
    return r;
  }

  void wrunlock() { return uv_rwlock_wrunlock(&_rwlock); }
};

// --

class Sem {
  uv::SemT _sem;

public:
  Sem(unsigned int value) {
    int r = uv_sem_init(&_sem, value);
    UVP_LOG_ERROR_EXIT(r);
  }
  virtual ~Sem() { uv_sem_destroy(&_sem); }

  void post() { return uv_sem_post(&_sem); }
  void wait() { return uv_sem_wait(&_sem); }
  int tryWait() {
    int r = uv_sem_trywait(&_sem);
    UVP_LOG_ERROR(r);
    return r;
  }
};

// --

class Cond {
  uv::CondT _cond;

public:
  Cond() {
    int r = uv_cond_init(&_cond);
    UVP_LOG_ERROR_EXIT(r);
  }
  virtual ~Cond() { uv_cond_destroy(&_cond); }

  void signal() { return uv_cond_signal(&_cond); }

  void broadcast() { return uv_cond_broadcast(&_cond); }

  void wait(Mutex *mutex) { return uv_cond_wait(&_cond, &mutex->_mutex); }

  int timedWait(Mutex *mutex, uint64_t timeout) {
    int r = uv_cond_timedwait(&_cond, &mutex->_mutex, timeout);
    UVP_LOG_ERROR(r);
    return r;
  }
};

// --

class Barrier {
  uv::BarrierT _barrier;

public:
  Barrier(unsigned int count) { int r = uv_barrier_init(&_barrier, count); }
  virtual ~Barrier() { uv_barrier_destroy(&_barrier); }

  int wait() { return uv_barrier_wait(&_barrier); }
};

// --

class Once {
  uv::OnceT _once;
  void (*_callback)(void);

public:
  Once(void (*cb)(void)) : _once(UV_ONCE_INIT) { _callback = cb; }
  virtual ~Once() {}

  void onlyOnce() { return uv_once(&_once, _callback); }
};

} // namespace uvp
