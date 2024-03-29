#pragma once

#include <functional>

#include "types.hpp"
#include "utilities.hpp"
#include "req.hpp"

// --

namespace uvp {

class Handle;

class Loop {
public:
  using RunMode = uv_run_mode;
  using LoopOperation = uv_loop_option;

  using WalkCallback = std::function<void(Handle *, void *)>;

protected:
  struct Impl {
    void *_data;
    WalkCallback _walkCallback;
  };
  Impl _impl;

  static void walk_callback(uv::HandleT *handle, void *arg) {
    auto h = (Handle *)uv_handle_get_data(handle);
    auto l = uv_handle_get_loop(handle);
    auto p = (Loop *)uv_loop_get_data(l);
    if (p->_impl._walkCallback) {
      p->_impl._walkCallback((Handle *)handle, arg);
    } else {
      UVP_ASSERT(false);
    }
  }

public:
  static size_t size() { return uv_loop_size(); }

  Loop() {}
  virtual ~Loop() {}

  virtual uv::LoopT *loop() const = 0;

  int configure(std::initializer_list<LoopOperation> options) {
    int r = 0;
    for (auto ptr = options.begin(); ptr != options.end(); ptr++) {
      r = uv_loop_configure(loop(), *ptr);
      UVP_LOG_ERROR_RETURN(r);
    }
    return r;
  }

  int close() {
    int r = uv_loop_close(loop());
    UVP_LOG_ERROR(r);
    return r;
  }

  int run(RunMode mode) {
    int r = uv_run(loop(), mode);
    UVP_LOG_ERROR(r);
    return r;
  }

  bool alive() const { return uv_loop_alive(loop()); }

  void stop() { uv_stop(loop()); }

  // Get backend file descriptor. Only kqueue, epoll and event ports are
  // supported.
  int backendFd() const { return uv_backend_fd(loop()); }

  int backendTimeout() const { return uv_backend_timeout(loop()); }

  uint64_t now() const { return uv_now(loop()); }

  void updateTime() { return uv_update_time(loop()); }

  void walk(const WalkCallback &cb, void *arg) {
    _impl._walkCallback = cb;
    uv_walk(loop(), walk_callback, arg);
  }

  int fork() {
    int r = uv_loop_fork(loop());
    UVP_LOG_ERROR(r);
    return r;
  }

  void *data() const { return _impl._data; }

  void *data(void *data) {
    _impl._data = data;
    return data;
  }

  // print handles
  void printAllHandles(FILE* file) {
    uv_print_all_handles(loop(), file);
  }

  void printActiveHandles(FILE* file) {
    uv_print_active_handles(loop(), file);
  }

  // Thread pool work scheduling
  int queueWork(Work *req, const Work::WorkCallback &work_cb,
                const Work::AfterWorkCallback &afterwork_cb) {
    UVP_ASSERT(work_cb && afterwork_cb);
    req->_impl._workCallback = work_cb;
    req->_impl._afterWorkCallback = afterwork_cb;

    int r = uv_queue_work(loop(), req->work(), Work::work_callback,
                          Work::afterwork_callback);
    UVP_LOG_ERROR(r);
    return r;
  }

  // dns functions
  int getAddrInfo(Getaddrinfo *req, const Getaddrinfo::Callback &cb,
                  const char *node, const char *service,
                  const addrinfo *hints) {
    UVP_ASSERT(cb);
    req->_impl._callback = cb;

    int r = uv_getaddrinfo(loop(), req->getaddrinfo(), Getaddrinfo::callback,
                           node, service, hints);
    UVP_LOG_ERROR(r);
    return r;
  }

  int getNameInfo(Getnameinfo* req, const Getnameinfo::Callback& cb, const struct sockaddr* addr, int flags) {
    UVP_ASSERT(cb);
    req->_impl._callback = cb;

    int r = uv_getnameinfo(loop(), req->getnameinfo(), Getnameinfo::callback, addr, flags);
    UVP_LOG_ERROR(r);
    return r;
  }
  
  // filesystem operations
  int fsClose(Fs *req, uv::File file, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_close(loop(), req->fs(), file, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_close(loop(), req->fs(), file, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsOpen(Fs *req, const char *path, int flags, int mode,
             const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_open(loop(), req->fs(), path, flags, mode, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_open(loop(), req->fs(), path, flags, mode, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsRead(Fs *req, uv::File file, const uv::BufT bufs[], UnsignedInt nbufs,
             Long offset, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_read(loop(), req->fs(), file, bufs, nbufs, offset, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_read(loop(), req->fs(), file, bufs, nbufs, offset,
                     Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsUnlink(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_unlink(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_unlink(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsWrite(Fs *req, uv::File file, const uv::BufT bufs[], UnsignedInt nbufs,
              Long offset, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_write(loop(), req->fs(), file, bufs, nbufs, offset, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_write(loop(), req->fs(), file, bufs, nbufs, offset,
                      Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsMkdir(Fs *req, const char *path, int mode, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_mkdir(loop(), req->fs(), path, mode, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_mkdir(loop(), req->fs(), path, mode, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsMkdtemp(Fs *req, const char *tpl, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_mkdtemp(loop(), req->fs(), tpl, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_mkdtemp(loop(), req->fs(), tpl, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsRmdir(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_rmdir(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_rmdir(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsScandir(Fs *req, const char *path, int flags, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_scandir(loop(), req->fs(), path, flags, nullptr);
    } else {
      req->_impl._callback = cb;
      int r = uv_fs_scandir(loop(), req->fs(), path, flags, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsStat(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_stat(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_stat(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFstat(Fs *req, uv::File file, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_fstat(loop(), req->fs(), file, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_fstat(loop(), req->fs(), file, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsLstat(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_lstat(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_lstat(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsRename(Fs *req, const char *path, const char *new_path,
               const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_rename(loop(), req->fs(), path, new_path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_rename(loop(), req->fs(), path, new_path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFsync(Fs *req, uv::File file, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_fsync(loop(), req->fs(), file, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_fsync(loop(), req->fs(), file, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFdatasync(Fs *req, uv::File file, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_fdatasync(loop(), req->fs(), file, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_fdatasync(loop(), req->fs(), file, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFtruncate(Fs *req, uv::File file, Long offset, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_ftruncate(loop(), req->fs(), file, offset, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_ftruncate(loop(), req->fs(), file, offset, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsCopyfile(Fs *req, const char *path, const char *new_path, int flags,
                 const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_copyfile(loop(), req->fs(), path, new_path, flags, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_copyfile(loop(), req->fs(), path, new_path, flags,
                         Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsSendfile(Fs *req, uv::File out_fd, uv::File in_fd, Long in_offset,
                 size_t length, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_sendfile(loop(), req->fs(), out_fd, in_fd, in_offset, length,
                         nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_sendfile(loop(), req->fs(), out_fd, in_fd, in_offset, length,
                         Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsAccess(Fs *req, const char *path, int mode, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_access(loop(), req->fs(), path, mode, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_access(loop(), req->fs(), path, mode, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsChmod(Fs *req, const char *path, int mode, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_chmod(loop(), req->fs(), path, mode, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_chmod(loop(), req->fs(), path, mode, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFchmod(Fs *req, uv::File file, int mode, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_fchmod(loop(), req->fs(), file, mode, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_fchmod(loop(), req->fs(), file, mode, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsUtime(Fs *req, const char *path, double atime, double mtime,
              const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_utime(loop(), req->fs(), path, atime, mtime, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_utime(loop(), req->fs(), path, atime, mtime, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFutime(Fs *req, uv::File file, double atime, double mtime,
               const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_futime(loop(), req->fs(), file, atime, mtime, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_futime(loop(), req->fs(), file, atime, mtime, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsLink(Fs *req, const char *path, const char *new_path,
             const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_link(loop(), req->fs(), path, new_path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_link(loop(), req->fs(), path, new_path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsSymlink(Fs *req, const char *path, const char *new_path, int flags,
                const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_symlink(loop(), req->fs(), path, new_path, flags, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_symlink(loop(), req->fs(), path, new_path, flags, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsReadlink(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_readlink(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_readlink(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsRealpath(Fs *req, const char *path, const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_realpath(loop(), req->fs(), path, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_realpath(loop(), req->fs(), path, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsChown(Fs *req, const char *path, uv::UidT uid, uv::GidT gid,
              const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_chown(loop(), req->fs(), path, uid, gid, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_chown(loop(), req->fs(), path, uid, gid, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsFchown(Fs *req, uv::File file, uv::UidT uid, uv::GidT gid,
               const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_fchown(loop(), req->fs(), file, uid, gid, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_fchown(loop(), req->fs(), file, uid, gid, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }

  int fsLchown(Fs *req, const char *path, uv::UidT uid, uv::GidT gid,
               const Fs::Callback &cb) {
    int r = 0;
    if (!cb) {
      r = uv_fs_lchown(loop(), req->fs(), path, uid, gid, nullptr);
    } else {
      req->_impl._callback = cb;
      r = uv_fs_lchown(loop(), req->fs(), path, uid, gid, Fs::callback);
    }
    UVP_LOG_ERROR(r);
    return r;
  }
};

class LoopObject : public Loop {
  mutable uv::LoopT _loop;

public:
  LoopObject() {
    int r = uv_loop_init(&_loop);
    UVP_LOG_ERROR_EXIT(r);
    uv_loop_set_data(loop(), this);
  }
  virtual ~LoopObject() {}

  virtual uv::LoopT *loop() const override { return &_loop; }
};

} // namespace uvp
