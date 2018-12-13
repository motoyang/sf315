#include <utilites.hpp>
#include <uv.hpp>
#include <req.hpp>

// --

struct ReqI::Impl {
  void* _data;
};

// --

size_t ReqI::size(ReqI::ReqType type) { return uv_req_size(type); }

const char *ReqI::name(ReqI::ReqType type) { return uv_req_type_name(type); }

int ReqI::cancel() {
  int r = uv_cancel(getReq());
  LOG_IF_ERROR(r);
  return r;
}

void *ReqI::data() const { return _impl->_data; }

void *ReqI::data(void *data) {
  _impl->_data = data;
  return data;
}

ReqI::ReqType ReqI::type() const { return uv_req_get_type(getReq()); }

ReqI::ReqI(): _impl(std::make_unique<ReqI::Impl>()) {}

ReqI::~ReqI() {}

// --

struct WorkI::Impl {
  WorkCallback _workCallback;
  AfterWorkCallback _afterWorkCallback;

  static void work_callback(uv_work_t *req);
  static void afterwork_callback(uv_work_t *req, int status);
};

void WorkI::Impl::work_callback(uv_work_t *req) {
  auto p = (WorkI *)uv_req_get_data((uv_req_t *)req);
  if (p->_impl->_workCallback) {
    p->_impl->_workCallback();
  }
}

void WorkI::Impl::afterwork_callback(uv_work_t *req, int status) {
  auto p = (WorkI *)uv_req_get_data((uv_req_t *)req);
  if (p->_impl->_afterWorkCallback) {
    p->_impl->_afterWorkCallback(status);
  }
}

WorkI::WorkI() : _impl(std::make_unique<WorkI::Impl>()) {}

WorkI::~WorkI() {}

int WorkI::queue(LoopI *from) {
  int r = uv_queue_work(from->getLoop(), getWork(), Impl::work_callback,
                        Impl::afterwork_callback);
  LOG_IF_ERROR(r);
  return r;
}

void WorkI::workCallback(const WorkI::WorkCallback &cb) {
  _impl->_workCallback = cb;
}

WorkI::WorkCallback WorkI::workCallback() const { return _impl->_workCallback; }

void WorkI::afterWorkCallback(const WorkI::AfterWorkCallback &cb) {
  _impl->_afterWorkCallback = cb;
}

WorkI::AfterWorkCallback WorkI::afterWorkCallback() const {
  return _impl->_afterWorkCallback;
}

// --

uv_req_t *WorkT::getReq() const { return (uv_req_t *)&_work; }

uv_work_t *WorkT::getWork() const { return (uv_work_t *)&_work; }

WorkT::WorkT() {
  uv_req_set_data(getReq(), this);
}

WorkT::~WorkT() {}
