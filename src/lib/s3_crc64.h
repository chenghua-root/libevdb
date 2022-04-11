#ifndef  S3_LIB_CRC64_H_
#define  S3_LIB_CRC64_H_

#include <stdint.h>
#include <string.h>

#define S3_DEFAULT_CRC64_POLYNOM 0xD800000000000000ULL

typedef uint64_t (*S3CRC64Func)(uint64_t, const char *, int64_t);
extern S3CRC64Func s3_crc64_sse42_func;

/**
  * create the crc64_table and optimized_crc64_table for calculate
  * must be called before s3_crc64
  * if not, the return value of s3_crc64 will be undefined
  * @param crc64 polynom
  */
void s3_init_crc64_table(const uint64_t polynom);

/**
  * Processes a multiblock of a CRC64 calculation.
  *
  * @returns Intermediate CRC64 value.
  * @param   uCRC64  Current CRC64 intermediate value.
  * @param   pv      The data block to process.
  * @param   cb      The size of the data block in bytes.
  */
uint64_t s3_crc64_again(uint64_t uCRC64, const void *pv, int64_t cb) ;

//Calculates CRC64 using CPU instructions.
static inline uint64_t s3_crc64_sse42_again(uint64_t uCRC64, const void *pv, int64_t cb)
{
  return (*s3_crc64_sse42_func)(uCRC64, (const char *)(pv), cb);
}

/**
  * Calculate CRC64 for a memory block.
  *
  * @returns CRC64 for the memory block.
  * @param   pv      Pointer to the memory block.
  * @param   cb      Size of the memory block in bytes.
  */
uint64_t s3_crc64(const void *pv, int64_t cb);

//Calculates CRC64 using CPU instructions.
static inline uint64_t s3_crc64_sse42(const void *pv, int64_t cb)
{
  return (*s3_crc64_sse42_func)(0, (const char *)(pv), cb);
}

/**
  * Get the static CRC64 table. This function is only used for testing purpose.
  *
  */
const uint64_t *s3_get_crc64_table();

#endif
