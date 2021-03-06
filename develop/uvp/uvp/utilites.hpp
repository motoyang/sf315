#include <cassert>
#include <nanolog/nanolog.hpp>

#define UVP_ASSERT(f) assert(f)

#define LOG_UV_ERROR(e) log_error(e, __FILE__, __func__, __LINE__)

#define LOG_IF_ERROR(e)                                                        \
  do {                                                                         \
    if (e) {                                                                   \
      LOG_UV_ERROR(e);                                                         \
    }                                                                          \
  } while (false)

#define LOG_IF_ERROR_RETURN(e)                                                 \
  do {                                                                         \
    if (e) {                                                                   \
      LOG_UV_ERROR(e);                                                         \
      return (e);                                                              \
    }                                                                          \
  } while (false)

#define LOG_IF_ERROR_EXIT(e)                                                   \
  do {                                                                         \
    if (e) {                                                                   \
      LOG_UV_ERROR(e);                                                         \
      std::exit(e);                                                            \
    }                                                                          \
  } while (false)

#define CHEKC_FUN_RETURN(f)                                                    \
  do {                                                                         \
    int e = f;                                                                 \
    LOG_IF_ERROR(e);                                                           \
  } while (false)

#define CHEKC_FUN_RETURN_EXIT(f)                                               \
  do {                                                                         \
    int e = f;                                                                 \
    LOG_IF_ERROR_EXIT(e);                                                      \
  } while (false)

#define LOG_NOMEM_EXIT(p)                                                      \
  do {                                                                         \
    if (!(p)) {                                                                \
      LOG_UV_ERROR(UV_ENOMEM);                                                 \
      std::exit(UV_ENOMEM);                                                    \
    }                                                                          \
  } while (false)

#define LOG_CONDITION(condition, err)                                          \
  do {                                                                         \
    if (condition) {                                                           \
      LOG_UV_ERROR(err);                                                       \
    }                                                                          \
  } while (false)

#define LOG_CONDITION_EXIT(condition, err)                                     \
  do {                                                                         \
    if (condition) {                                                           \
      LOG_UV_ERROR(err);                                                       \
      std::exit(err);                                                          \
    }                                                                          \
  } while (false)

#define LOG_TRACK (LOG_INFO << " haha...")

#define COUNT_OF(t) (sizeof(t) / sizeof(t[0]))

// --

void log_error(int err, const char *fn, const char *fun, uint64_t ln);
std::ostream &hex_dump(std::ostream &o, std::string const &v);
const char *demangle(const char *name);

// --
