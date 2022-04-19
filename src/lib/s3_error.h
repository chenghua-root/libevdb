#ifndef S3_LIB_ERROR_H_
#define S3_LIB_ERROR_H_

#define _as_str(q, ...)   #q,
#define _as_const(q, ...)  q,
#define _errs(_render)                                                        \
  _render(S3_ERR_ERR, -1000)                                                  \
  _render(S3_ERR_AGAIN)                                                       \
  _render(S3_ERR_CHKSUM)                                                      \
  _render(S3_ERR_INVALID_ARG)                                                 \
  _render(S3_ERR_INVALID_STATE)                                               \
  _render(S3_ERR_INVALID_RPC)                                                 \
  _render(S3_ERR_NOT_FOUND)                                                   \
  _render(S3_ERR_NET_AGAIN)                                                   \
  _render(S3_ERR_NET_ABORT)                                                   \
  _render(S3_ERR_INIT_TWICE)                                                  \
  _render(S3_ERR_OUT_OF_MEM)                                                  \
  _render(S3_ERR_TIMEOUT)

#define S3_FAIL S3_ERR_ERR

enum S3Error {
  S3_OK = 0,
  /*
   * Start internal error number from -1000 in order to avoid overriding system
   * errno range(-1 to -122).
   */
  _S3_ERR_START = -1001,
  _errs(_as_const)
};

static const char *s3_error_string_ok = "S3_OK";
static const char *s3_error_string_unknown = "S3_ERR_UNKNOWN";

static const char *s3_error_string_[] = {
  _errs(_as_str)
};

static inline const char *s3_err_str(int rc) {
  if (rc == 0) {
    return s3_error_string_ok;
  }

  rc = rc - S3_ERR_ERR;
  if (rc < 0 || rc >= sizeof(s3_error_string_)) {
    return s3_error_string_unknown;
  }

  return s3_error_string_[ rc ];
}

#undef _errs
#undef _as_str
#undef _as_const

#endif
