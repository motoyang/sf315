#pragma once

#include <uv.h>

#include <functional>
#include <memory>
#include <initializer_list>

// --

class ReqI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_req_t *getReq() const = 0;

public:
  using ReqType = uv_req_type;

  static size_t size(ReqType type);
  static const char *name(ReqType type);

  ReqI();
  virtual ~ReqI();

  int cancel();
  ReqType type() const;
  void *data(void *data);
  void *data() const;
};

// --

class LoopT;
class WorkI : public ReqI {
protected:
  class Impl;
  std::unique_ptr<Impl> _impl;

  virtual uv_work_t *getWork() const = 0;

public:
  using WorkCallback = std::function<void()>;
  using AfterWorkCallback = std::function<void(int)>;

  WorkI();
  virtual ~WorkI();

  int queue(LoopT *from);

  void workCallback(const WorkCallback &cb);
  WorkCallback workCallback() const;
  void afterWorkCallback(const AfterWorkCallback &cb);
  AfterWorkCallback afterWorkCallback() const;
};

class WorkT : public WorkI {
  uv_work_t _work;

protected:
  virtual uv_req_t *getReq() const override;
  virtual uv_work_t *getWork() const override;

public:
  WorkT();
  virtual ~WorkT();
};
