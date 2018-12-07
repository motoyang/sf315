#include <req.hpp>
#include <uv.hpp>
#include <utilites.hpp>

// --

size_t ReqI::size(ReqI::ReqType type) { return uv_req_size(type); }

const char *ReqI::typeName(ReqI::ReqType type) {
  return uv_req_type_name(type);
}

int ReqI::cancel() {
  int r = uv_cancel(getReq());
  LOG_IF_ERROR(r);
  return r;
}

void *ReqI::data() const { return uv_req_get_data(getReq()); }

void *ReqI::data(void *data) {
  uv_req_set_data(getReq(), data);
  return data;
}

ReqI::ReqType ReqI::type() const { return uv_req_get_type(getReq()); }

// --

uv_req_t *ReqT::getReq() const { return (uv_req_t *)&_req; }

// --

StreamT *ShutdownI::stream() {
  auto p = (StreamT *)uv_handle_get_data((uv_handle_t *)getShutdown()->handle);
  return p;
}

// --

uv_req_t *ShutdownT::getReq() const { return (uv_req_t *)&_shutdown; }

uv_shutdown_t *ShutdownT::getShutdown() const {
  return (uv_shutdown_t *)&_shutdown;
}

// --

StreamT *WriteI::stream() {
  auto p = (StreamT *)uv_handle_get_data((uv_handle_t *)getWrite()->handle);
  return p;
}

// --

uv_req_t *WriteT::getReq() const { return (uv_req_t *)&_write; }

uv_write_t *WriteT::getWrite() const { return (uv_write_t *)&_write; }
