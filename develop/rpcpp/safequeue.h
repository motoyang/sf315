#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

// --

template <typename T> class threadsafe_queue {
private:
  mutable std::mutex mut;
  std::queue<T> data_queue;
  std::condition_variable data_cond;

public:
  threadsafe_queue() {}

  operator=(threadsafe_queue const& other) {
    std::lock_guard<std::mutex> lk(other.mut);
    data_queue = other.data_queue;
  }

  // 入队操作
  void push(T&& new_value) {
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(std::forward<T>(new_value));
    data_cond.notify_one();
  }

  // 直到有元素可以删除为止
  void wait_and_pop(T &value) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    value = data_queue.front();
    data_queue.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [this] { return !data_queue.empty(); });
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }

  // 不管有没有队首元素直接返回
  bool try_pop(T &value) {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return false;
    value = data_queue.front();
    data_queue.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(mut);
    if (data_queue.empty())
      return std::shared_ptr<T>();
    std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
    data_queue.pop();
    return res;
  }
  bool empty() const {
    std::lock_guard<std::mutex> lk(mut);
    return data_queue.empty();
  }
};

// --

template <typename T> class threadsafe_queue2 {
public:
  threadsafe_queue2() : head(new node), tail(head.get()) {}

  threadsafe_queue2(const threadsafe_queue2 &) = delete;

  threadsafe_queue2 operator=(const threadsafe_queue2 &) = delete;

  ~threadsafe_queue2() = default;

  std::shared_ptr<T> try_pop() {
    std::unique_lock<std::mutex> ulkh(head_mut);
    if (head.get() == get_tail()) {
      return nullptr;
    }
    auto old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head->data;
  }

  //此版本存在最后一个元素加入后，可能一直阻塞在wait中，下面一个函数不存在此问题
  /*std::shared_ptr<T> wait_and_pop()
  {
      std::unique_lock<std::mutex> ulkh(head_mut);
      data_cond.wait(ulkh, [&]()
          {
              node *old_tail=get_tail();
              return head.get() != old_tail;
          });
      auto old_head = std::move(head);
      head = std::move(old_head->next);
      return old_head->data;
  }*/

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> ulkh(head_mut);
    {
      std::unique_lock<std::mutex> ulkt(tail_mut);
      data_cond.wait(ulkt, [&]() { return head.get() != tail; });
    }
    auto old_head = std::move(head);
    head = std::move(old_head->next);
    return old_head->data;
  }

  void push(T &&t) {
    std::shared_ptr<T> new_data(std::make_shared<T>(std::forward<T>(t)));
    std::unique_ptr<node> new_tail(new node);
    node *p = new_tail->get();
    {
      std::unique_lock<std::mutex> ulkt(tail_mut);
      tail->data = new_data;
      tail->next = std::move(new_tail);
      tail = p;
    }
    data_cond.notify_one();
  }

private:
  struct node {
    std::shared_ptr<T> data;
    std::unique_ptr<node> next;
  };

  std::mutex head_mut;
  std::mutex tail_mut;
  std::unique_ptr<node> head;
  node *tail;
  std::condition_variable data_cond;

private:
  node *get_tail() {
    std::unique_lock<std::mutex> ulkt(tail_mut);
    return tail;
  }
};

template <typename T> class threadsafe_queue2<T *> {
public:
  threadsafe_queue2() : head(new node), tail(head) {}

  threadsafe_queue2(const threadsafe_queue2 &) = delete;

  threadsafe_queue2 operator=(const threadsafe_queue2 &) = delete;

  ~threadsafe_queue2() {
    node *pre;
    for (; head != tail;) {
      pre = head;
      head = head->next;
      delete pre;
    }
    delete tail;
  }

  T *try_pop() {
    node *old_head = nullptr;
    {
      std::unique_lock<std::mutex> ulkh(head_mut);
      if (head == get_tail())
        return nullptr;
      old_head = head;
      head = head->next;
    }
    T *data = old_head->data;
    delete old_head;
    return data;
  }

  //此版本存在最后一个元素加入后，可能一直阻塞在wait中，下面一个函数不存在此问题

  /*T* wait_and_pop()
  {
      node *old_head = nullptr;
      {
          std::unique_lock<std::mutex> ulkh(head_mut);
          data_cond.wait(ulkh, [&]()
              {
                  node *old_tail = get_tail();
                  return head != old_tail;
              });
          old_head = head;
          head = head->next;
      }
      T *data = old_head->data;
      delete old_head;
      return data;
  }*/

  T *wait_and_pop() {
    node *old_head = nullptr;
    {
      std::unique_lock<std::mutex> ulkh(head_mut);
      {
        std::unique_lock<std::mutex> ulkt(tail_mut);
        data_cond.wait(ulkt, [&]() { return head != tail; });
      }
      old_head = head;
      head = head->next;
    }
    T *data = old_head->data;
    delete old_head;
    return data;
  }

  void push(T *t) {
    node *new_tail = new node;
    {
      std::unique_lock<std::mutex> ulkt(tail_mut);
      tail->data = t;
      tail->next = new_tail;
      tail = new_tail;
    }
    data_cond.notify_one();
  }

private:
  struct node {
    T *data;
    node *next;
  };

  std::mutex head_mut;
  std::mutex tail_mut;
  node *head;
  node *tail;
  std::condition_variable data_cond;

private:
  node *get_tail() {
    std::unique_lock<std::mutex> ulkt(tail_mut);
    return tail;
  }
};
