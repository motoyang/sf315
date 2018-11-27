#pragma once

#include <vector>
#include <atomic>
#include <future>

#include "concurrentqueue.h"

namespace rpcpp {

using namespace std;

// --

class ThreadPool {
  using Task = function<void()>;
  vector<thread> _pool;
  moodycamel::ConcurrentQueue<Task> _tasks;
  atomic<bool> _running {true};
  atomic<int> _idle {0};

public:
  inline ThreadPool(unsigned short size) { addThread(size); }

  inline ~ThreadPool() {
    //			stop();
    //			join_all();
  }

public:
  template <class F, class... Args>
  auto commit(F &&f, Args &&... args) -> future<decltype(f(args...))> {
    using RetType = decltype(f(args...));
    if (!_running) {
      return future<RetType>();
    }

    auto task = make_shared<packaged_task<RetType()>>(
        bind(forward<F>(f), forward<Args>(args)...));
    future<RetType> future = task->get_future();
    _tasks.enqueue([task]() { (*task)(); });

    return future;
  }

  void join_all() {
    for_each(_pool.begin(), _pool.end(), mem_fn(&thread::join));
  }

  void stop() { _running.store(false); }
  int idleCount() { return _idle.load(); }
  int threadCount() { return _pool.size(); }

private:
  void addThread(unsigned short size) {
    for (int i = 0; i < size; ++i) {
      _pool.emplace_back([this]() {
        do {
          Task task;
          if (_tasks.try_dequeue(task)) {
            _idle.fetch_add(1);
            task();
            _idle.fetch_sub(1);
          }
          std::this_thread::sleep_for(chrono::milliseconds(100));
        } while (_running || _tasks.size_approx() > 0);
      });
    }
  }
};

// --

} // namespace rpcpp
