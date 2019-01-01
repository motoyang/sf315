#pragma once

#include <details/types.hpp>
#include <details/utilities.hpp>

#include <functional>

namespace uvp {

struct Req {
  using ReqType = uv::ReqType;

  virtual uv::ReqT *req() const = 0;

  static size_t size(ReqType type) { return uv_req_size(type); }

  static const char *name(ReqType type) { return uv_req_type_name(type); }

  struct Impl {
    void *_data;
  };
  Impl _impl;

  Req() {}
  virtual ~Req() {}

  int cancel() {
    int r = uv_cancel(req());
    UVP_LOG_ERROR(r);
    return r;
  }

  ReqType type() const { return uv_req_get_type(req()); }

  void *data(void *data) {
    _impl._data = data;
    return data;
  }
  
  void *data() const { return _impl._data; }
};

// --

struct Work : public Req {
  using WorkCallback = std::function<void(Work *)>;
  using AfterWorkCallback = std::function<void(Work *, int)>;

  struct Impl {
    WorkCallback _workCallback;
    AfterWorkCallback _afterWorkCallback;
  };
  Impl _impl;

  static void work_callback(uv::WorkT *req) {
    auto p = (Work *)uv_req_get_data((uv::ReqT *)req);
    if (p->_impl._workCallback) {
      p->_impl._workCallback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

  static void afterwork_callback(uv::WorkT *req, int status) {
    auto p = (Work *)uv_req_get_data((uv::ReqT *)req);
    if (p->_impl._afterWorkCallback) {
      p->_impl._afterWorkCallback(p, status);
    } else {
      UVP_ASSERT(false);
    }
  }

  virtual uv::WorkT *work() const = 0;

  Work() {}
  virtual ~Work() {}
};

struct WorkReq : public Work {
  virtual uv::ReqT *req() const override { return (uv::ReqT *)&_work; }
  virtual uv::WorkT *work() const override { return &_work; }

  WorkReq() { uv_req_set_data(req(), this); }
  virtual ~WorkReq() {}

private:
  mutable uv::WorkT _work = {0};
};

// --

struct Fs : public Req {
  using Callback = std::function<void(Fs *)>;
  using Type = uv::FsType;
  using Stat = uv::StatT;

  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::FsT *req) {
    auto p = (Fs *)uv_req_get_data((uv::ReqT *)req);
    if (p->_impl._callback) {
      p->_impl._callback(p);
    } else {
      UVP_ASSERT(false);
    }
  }

  virtual uv::FsT *fs() const = 0;

  Fs(){};
  virtual ~Fs() {}

  int fsScandirNext(uv::DirentT *ent) {
    int r = uv_fs_scandir_next(fs(), ent);
    UVP_LOG_ERROR(r);
    return r;
  }

  Type getType() const { return uv_fs_get_type(fs()); }

  ssize_t getResult() const { return uv_fs_get_result(fs()); }

  void *getPtr() const { return uv_fs_get_ptr(fs()); }

  const char *getPath() const { return uv_fs_get_path(fs()); }

  Stat *getStatbuf() { return uv_fs_get_statbuf(fs()); }
};

struct FsReq : public Fs {
  virtual uv::ReqT *req() const override { return (uv::ReqT *)&_fs; }
  virtual uv::FsT *fs() const override { return &_fs; }

  FsReq() { uv_req_set_data(req(), this); }
  virtual ~FsReq() { uv_fs_req_cleanup(&_fs); }

private:
  mutable uv::FsT _fs = {0};
};

// --

struct Getaddrinfo : public Req {
  using Callback = std::function<void(Getaddrinfo *, int, addrinfo *)>;

  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::GetaddrinfoT *req, int status, addrinfo *res) {
    auto p = (Getaddrinfo *)uv_req_get_data((uv::ReqT *)req);
    if (p->_impl._callback) {
      p->_impl._callback(p, status, res);
    } else {
      UVP_ASSERT(false);
    }
    uv_freeaddrinfo(res);
  }

  virtual uv::GetaddrinfoT *getaddrinfo() const = 0;
};

struct GetaddrinfoReq : public Getaddrinfo {
  GetaddrinfoReq() { uv_req_set_data(req(), this); }
  virtual ~GetaddrinfoReq() {}

  virtual uv::ReqT *req() const override { return (uv::ReqT *)&_addr; }

  virtual uv::GetaddrinfoT *getaddrinfo() const override { return &_addr; }

private:
  mutable uv::GetaddrinfoT _addr = {0};
};

// --

struct Getnameinfo : public Req {
  using Callback =
      std::function<void(Getnameinfo *, int, const char *, const char *)>;

  struct Impl {
    Callback _callback;
  };
  Impl _impl;

  static void callback(uv::GetnameinfoT *req, int status, const char *hostname,
                       const char *service) {
    auto p = (Getnameinfo *)uv_req_get_data((uv::ReqT *)req);
    if (p->_impl._callback) {
      p->_impl._callback(p, status, hostname, service);
    } else {
      UVP_ASSERT(false);
    }
  }

  virtual uv::GetnameinfoT *getnameinfo() const = 0;
};

struct GetnameinfoReq : public Getnameinfo {
  virtual uv::ReqT *req() const override { return (uv::ReqT *)&_name; }
  virtual uv::GetnameinfoT *getnameinfo() const override { return &_name; }

  GetnameinfoReq() { uv_req_set_data(req(), this); }
  virtual ~GetnameinfoReq() {}

private:
  mutable uv::GetnameinfoT _name;
};

} // namespace uvp
