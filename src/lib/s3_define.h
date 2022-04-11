#ifndef S3_LIB_DEFINE_
#define S3_LIB_DEFINE_

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define S3_MAX_LOCAL_FILE_NAME_LENGTH 16384
#define S3_LITERAL_NULL "(NULL)"

/* s3 memory allocate functions */
#define s3_free(ptr)              free(ptr)
#define s3_malloc(size)           malloc(size)
#define s3_calloc(cnt, size)      calloc((cnt), (size))
#define s3_realloc(ptr, size)     realloc((ptr), (size))

/* struct/field operation */
#define s3_offsetof(tp, field) ( (size_t)&((tp*)NULL)->field )
#define s3_by_offset(p, offset) ( &((char*)(p))[offset] )
#define s3_owner(p, tp, field) (tp*)s3_by_offset(p, -s3_offsetof(tp, field))

#define s3_no_elts(arr) (sizeof(arr) / sizeof((arr)[0]))
#define s3_n_arr(tp, ...) \
    (sizeof((tp[]){__VA_ARGS__}) / sizeof(tp)), (tp[]){__VA_ARGS__}

#define s3_safe_close(fd) {if((fd)>=0){close((fd));(fd)=-1;}}
#define s3_memcpy(a, b, size)  memcpy((a), (b), (size))
#define s3_memcmp(a, b, size)  memcmp((a), (b), (size))
#define s3_strncmp(a, b, size) memcmp((a), (b), (size))

typedef uint8_t bool;
#define true ((uint8_t)1)
#define false ((uint8_t)0)

/* S3 data type names */
typedef void *S3Ptr;
typedef const void *S3ConstPtr;

typedef int (*S3EqualFunc) (S3ConstPtr a, S3ConstPtr b);
typedef int (*S3CompareFunc) (S3ConstPtr a, S3ConstPtr b);
typedef int (*S3CompareDataFunc) (S3ConstPtr a, S3ConstPtr b, S3Ptr data);

typedef void *(*S3FuncCB)(void*);

typedef struct {
  S3FuncCB cb_func;
  void *cb_arg;
} S3Callback;

/* s3 common macros */
#define s3_max(a, b)  (((a) > (b)) ? (a) : (b))
#define s3_min(a, b)  (((a) < (b)) ? (a) : (b))
#define s3_abs(a)     (((a) < 0) ? -(a) : (a))
#define s3_align_ptr(p, a)        (uint8_t*)(((uintptr_t)(p) + ((uintptr_t) (a) - 1)) & ~((uintptr_t) (a) - 1))
#define s3_align(d, a)            (((d) + ((a) - 1)) & ~((a) - 1))
#define s3_normalization(n) ((n) > 0) ? 1 : (((n) < 0) ? -1 : 0 )
#define s3_compare(a, b) ((a) > (b)) ? 1 : (((a) < (b)) ? -1 : 0 )

/* remove this: */
#define s3_norm_cmp(a, b) ((a) > (b)                                          \
                           ? 1                                                \
                           : ((a) < (b) ? -1 : 0))
#define s3_norm_cmp_return_ne(a, b)                                           \
  do {                                                                        \
    int ret = s3_norm_cmp(a, b);                                              \
    if (ret != 0) {                                                           \
      return ret;                                                             \
    }                                                                         \
  } while(0)

/* s3 likely and unlikely */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _S3_BOOLEAN_EXPR(expr)                                                \
  __extension__ ({                                                            \
          int _s3_boolean_var_;                                               \
          if (expr)                                                           \
             _s3_boolean_var_ = 1;                                            \
          else                                                                \
             _s3_boolean_var_ = 0;                                            \
          _s3_boolean_var_;                                                   \
       })
#define s3_likely(expr) (__builtin_expect (_S3_BOOLEAN_EXPR(expr), 1))
#define s3_unlikely(expr) (__builtin_expect (_S3_BOOLEAN_EXPR(expr), 0))
#else
#define s3_likely(expr) (expr)
#define s3_unlikely(expr) (expr)
#endif

/* s3 check arguments valid macros */

#define s3_arg1_(a, ...) a
#define s3_arg2_(a, b, ...) b
#define s3_arg3_(a, b, c, ...) c

#define s3_must_inited(_arg, ...) \
  do { \
    s3_must_be((_arg) != NULL, s3_arg3_(x, ##__VA_ARGS__, S3_ERR_INVALID_ARG, (void)0)); \
    s3_must_be((_arg)->inited_ == 1, s3_arg2_(x, ##__VA_ARGS__, (void)0)); \
  } while (0)

#define s3_check_inited(_arg, ...) \
  do { \
    s3_check_be((_arg) != NULL, s3_arg3_(x, ##__VA_ARGS__, S3_ERR_INVALID_ARG, (void)0)); \
    s3_check_be((_arg)->inited_ == 1, ##__VA_ARGS__); \
  } while (0)

#define s3_must_uninited(_arg, ...) \
  do { \
    s3_must_be((_arg) != NULL, s3_arg3_(x, ##__VA_ARGS__, S3_ERR_INVALID_ARG, (void)0)); \
    s3_must_be((_arg)->inited_ == 0, s3_arg2_(x, ##__VA_ARGS__, (void)0)); \
  } while (0)

#ifdef S3_DISABLE_CHECKS

#define s3_must_be(expr, ...) do{ (void)0; } while(0)

#else /* !S3_DISABLE_CHECKS */

#define s3_must_be(expr, ...) do {                                            \
    if (! s3_likely(expr)) {                                                  \
      S3_WARN("assertion fail: "#expr);                                      \
      return (void)0, ##__VA_ARGS__;                                          \
    }                                                                         \
  } while (0)

#define s3_check_be(expr, ...) do {                                            \
    if (! s3_likely(expr)) {                                                  \
      S3_DEBUG("assertion fail: "#expr);                                      \
      return (void)0, ##__VA_ARGS__;                                          \
    }                                                                         \
  } while (0)

#endif /* !S3_DISABLE_CHECKS */

#define s3_check_ret_none(expr, err_msg, ...)                                 \
  __s3_check_ret(S3_NONE, S3_TRACE, expr, err_msg, ##__VA_ARGS__)

#define s3_check_ret(expr, err_msg, ...)                                      \
  __s3_check_ret(S3_ERROR, S3_DEBUG, expr, err_msg, ##__VA_ARGS__)

#define s3_check_ret_warn(expr, err_msg, ...)                                 \
  __s3_check_ret(S3_WARN, S3_DEBUG, expr, err_msg, ##__VA_ARGS__)

#define s3_check_ret_info(expr, err_msg, ...)                                 \
  __s3_check_ret(S3_INFO, S3_DEBUG, expr, err_msg, ##__VA_ARGS__)

#define s3_check_ret_debug(expr, err_msg, ...)                                \
  __s3_check_ret(S3_DEBUG, S3_TRACE, expr, err_msg, ##__VA_ARGS__)

#define s3_check_ret_trace(expr, err_msg, ...)                                \
  __s3_check_ret(S3_TRACE, S3_TRACE, expr, err_msg, ##__VA_ARGS__)

#define s3_timeout_check_ret(expr, err_msg, ...)  do {                        \
  const int __timeout_expr_ret__ = (expr);                                    \
    if (S3_ERR_TIMEOUT == __timeout_expr_ret__ ||                             \
        S3_ERR_NET_RESPONSE_TIME_OUT == __timeout_expr_ret__) {               \
      s3_check_ret_warn(__timeout_expr_ret__, err_msg, ##__VA_ARGS__);        \
    } else {                                                                  \
      s3_check_ret(__timeout_expr_ret__, err_msg, ##__VA_ARGS__);             \
    }                                                                         \
} while(0)

#define s3_not_found_check_ret(expr, err_msg, ...)  do {                      \
  const int __not_found_expr_ret__ = (expr);                                  \
    if (S3_ERR_NOT_FOUND == __not_found_expr_ret__ ||                         \
        S3_ERR_REPLICA_NOT_FOUND == __not_found_expr_ret__ ) {                \
      s3_check_ret_warn(__not_found_expr_ret__, err_msg, ##__VA_ARGS__);      \
    } else {                                                                  \
      s3_check_ret(__not_found_expr_ret__, err_msg, ##__VA_ARGS__);           \
    }                                                                         \
} while(0)

#define s3_timeout_nf_check_ret(expr, err_msg, ...)  do {                     \
  const int __timeout_nf_expr_ret__ = (expr);                                 \
    if (S3_ERR_TIMEOUT == __timeout_nf_expr_ret__ ||                          \
        S3_ERR_NOT_FOUND == __timeout_nf_expr_ret__ ||                        \
        S3_ERR_REPLICA_NOT_FOUND == __timeout_nf_expr_ret__) {                \
      s3_check_ret_warn(__timeout_nf_expr_ret__, err_msg, ##__VA_ARGS__);     \
    } else {                                                                  \
      s3_check_ret(__timeout_nf_expr_ret__, err_msg, ##__VA_ARGS__);          \
    }                                                                         \
} while(0)



#ifdef S3_LIB
#define __s3_check_ret(_logfunc, _debug_logfunc, expr, err_msg, ...) do {     \
     const int __ret__ = (expr);                                              \
     if s3_unlikely(S3_OK != __ret__) {                                       \
         _logfunc("ERR: " err_msg ", " #expr " = %d: %s.",                    \
                        ##__VA_ARGS__, __ret__, s3_err_str(__ret__));         \
         goto exit;                                                           \
       };                                                                     \
     } while (0)
#else
#define __s3_check_ret(_logfunc, _debug_logfunc, expr, err_msg, ...) do {     \
     const int __ret__ = (expr);                                              \
     if s3_likely(S3_OK == __ret__) {                                         \
         _debug_logfunc("OK: " err_msg ", " #expr " = %d: %s.",               \
                        ##__VA_ARGS__, __ret__, s3_err_str(__ret__));         \
     } else {                                                                 \
         _logfunc("ERR: " err_msg ", " #expr " = %d: %s.",                    \
                        ##__VA_ARGS__, __ret__, s3_err_str(__ret__));         \
         goto exit;                                                           \
       };                                                                     \
     } while (0)
#endif

#define s3_exit(err_msg, ...) __s3_exit(S3_ERROR, err_msg, ##__VA_ARGS__)
#define s3_exit_warn(err_msg, ...) __s3_exit(S3_WARN, err_msg, ##__VA_ARGS__)
#define s3_exit_info(err_msg, ...) __s3_exit(S3_INFO, err_msg, ##__VA_ARGS__)
#define s3_exit_trace(err_msg, ...) __s3_exit(S3_TRACE, err_msg, ##__VA_ARGS__)
#define s3_exit_debug(err_msg, ...) __s3_exit(S3_DEBUG, err_msg, ##__VA_ARGS__)
#define s3_exit_none(err_msg, ...) __s3_exit(S3_NONE, err_msg, ##__VA_ARGS__)

#define __s3_exit(_logfunc, err_msg, ...) do {                                \
       _logfunc(err_msg, ##__VA_ARGS__ );                                     \
       goto exit;                                                             \
     } while(0)

#define s3_bug(...) raise(11)
#ifdef __cplusplus
}
#endif
#endif /* S3_LIB_DEFINE_ */
// vim:ts=8:sw=2:et
