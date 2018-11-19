#pragma once

bool initRep(int i);

// --

namespace rpcpp2 {

// --

class Task {
public:
  virtual void close() = 0;
};

// --

class TaskManager {
  std::map<std::string, std::unique_ptr<Task>> _tasks;

public:
  void AddTask(const std::string& n, std::unique_ptr<Task>&& t) {
    _tasks.insert(std::make_pair(n, std::forward<decltype(t)>(t)));
  };

  void stop() {
    for (auto& p: _tasks) {
      p.second->close();
    }
  }
};

// --

}
