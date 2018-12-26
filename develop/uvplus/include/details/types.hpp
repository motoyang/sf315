#pragma once

#include <uv.h>

#include <cstdint>
#include <string>
#include <unordered_map>

// --

namespace uvp {

typedef std::uint8_t UnsignedByte;
typedef std::int8_t Byte;
typedef std::uint16_t UnsignedShort;
typedef std::int16_t Short;
typedef std::uint32_t UnsignedInt;
typedef std::int32_t Int;
typedef std::uint64_t UnsignedLong;
typedef std::int64_t Long;
typedef float Float;
typedef double Double;
typedef std::intptr_t Pointer;
typedef std::uintptr_t UnsignedPointer;

// --

using FunType = std::string (*)(Pointer, const char *, std::size_t, std::size_t,
                                std::size_t);
using ValueType = std::pair<FunType, Pointer>;

// --

namespace uv {

using File = uv_file;

using BufT = uv_buf_t;
using OsFdT = uv_os_fd_t;
using OsSockT = uv_os_sock_t;
using DirentT = uv_dirent_t;
using UidT = uv_uid_t;
using GidT = uv_gid_t;
using StatT = uv_stat_t;
using PidT = uv_pid_t;
using UidT = uv_uid_t;
using GidT = uv_gid_t;
using StdioContainerT = uv_stdio_container_t;
using ProcessOptionsT = uv_process_options_t;


using InterfaceAddress = uv_interface_address_t;

using TtyMode = uv_tty_mode_t;

using HandleType = uv_handle_type;
using ReqType = uv_req_type;
using FsType = uv_fs_type;

using Membership = uv_membership;

using ReqT = uv_req_t;
using WorkT = uv_work_t;
using FsT = uv_fs_t;
using GetaddrinfoT = uv_getaddrinfo_t;
using GetnameinfoT = uv_getnameinfo_t;
using ConnectT = uv_connect_t;
using WriteT = uv_write_t;
using ShutdownT = uv_shutdown_t;
using UdpSendT = uv_udp_send_t;

using LoopT = uv_loop_t;
using HandleT = uv_handle_t;
using IdleT = uv_idle_t;
using TimerT = uv_timer_t;
using StreamT = uv_stream_t;
using PipeT = uv_pipe_t;
using TcpT = uv_tcp_t;
using TtyT = uv_tty_t;
using UdpT = uv_udp_t;
using AsyncT = uv_async_t;
using SignalT = uv_signal_t;
using ProcessT = uv_process_t;
using FsEventT = uv_fs_event_t;

using ThreadT = uv_thread_t;
using KeyT = uv_key_t;
using OnceT = uv_once_t;
using MutexT = uv_mutex_t;
using Rwlock = uv_rwlock_t;
using SemT = uv_sem_t;
using CondT = uv_cond_t;
using BarrierT = uv_barrier_t;
}

// --

enum class BufType : int {
  BUF_NULL_TYPE = 0,
  BUF_REQ_TYPE,
  BUF_REP_TYPE,
  BUF_ECHO_TYPE,
  BUF_RESOLVE_TYPE,
  BUF_TYPE_END
};

} // namespace uvp
