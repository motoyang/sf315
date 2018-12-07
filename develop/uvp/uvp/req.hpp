#pragma once

#include <uv.h>

#include <functional>
#include <memory>
#include <initializer_list>


class ReqI {
protected:
  virtual uv_req_t *getReq() const = 0;

public:
  using ReqType = uv_req_type;

  static size_t size(ReqType type);
  static const char *typeName(ReqType type);

  int cancel();
  void *data() const;
  void *data(void *data);
  ReqType type() const;
};

class ReqT : public ReqI {
  uv_req_t _req;

protected:
  virtual uv_req_t *getReq() const override;

public:
};

// --

class StreamT;
class ShutdownI : public ReqI {
protected:
  virtual uv_shutdown_t *getShutdown() const = 0;

public:
  StreamT* stream();
};

class ShutdownT: public ShutdownI {
  friend class StreamI;
  uv_shutdown_t _shutdown;

protected:
  virtual uv_req_t *getReq() const override;
  virtual uv_shutdown_t *getShutdown() const override;

public:
};

// --

class WriteI: public ReqI {
protected:
  virtual uv_write_t *getWrite() const = 0;

public:
  StreamT* stream();
};

class WriteT: public WriteI {
  friend class StreamI;
  uv_write_t _write;

  protected:
  virtual uv_req_t *getReq() const override;
  virtual uv_write_t *getWrite() const override;

public:
  
};