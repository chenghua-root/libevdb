#include <stdlib.h>
#include "lib/s3_crc64.h"


/*******************************************************************************
 *   Global Variables                                                           *
 *******************************************************************************/
/**
  * Lookup table (precomputed CRC64 values for each 8 bit string) computation
  * takes into account the fact that the reverse polynom has zeros in lower 8 bits:
  *
  * @code
  *    for (i = 0; i < 256; i++)
  *    {
  *        shiftRegister = i;
  *        for (j = 0; j < 8; j++)
  *        {
  *            if (shiftRegister & 1)
  *                shiftRegister = (shiftRegister >> 1) ^ Reverse_polynom;
  *            else
  *                shiftRegister >>= 1;
  *        }
  *        CRCTable[i] = shiftRegister;
  *    }
  * @endcode
  *
  */
#define CRC64_TABLE_SIZE 256
static uint64_t s_crc64_table[CRC64_TABLE_SIZE] = {0};
static uint16_t s_optimized_crc64_table[CRC64_TABLE_SIZE] = {0};

void __attribute__((constructor)) s3_global_init_crc64_table()
{
  s3_init_crc64_table(S3_DEFAULT_CRC64_POLYNOM);
}

void s3_init_crc64_table(const uint64_t polynom)
{
  for (uint64_t i = 0; i < CRC64_TABLE_SIZE; i++) {
    uint64_t shift = i;
    for (uint64_t j = 0; j < 8; j++) {
      if (shift & 1) {
        shift = (shift >> 1) ^ polynom;
      } else {
        shift >>= 1;
      }
    }
    s_crc64_table[i] = shift;
    s_optimized_crc64_table[i] = (int16_t)((shift >> 48) & 0xffff);
  }
}

#define DO_1_STEP(uCRC64, pu8) uCRC64 = s_crc64_table[(uCRC64 ^ *pu8++) & 0xff] ^ (uCRC64 >> 8);
#define DO_2_STEP(uCRC64, pu8)  DO_1_STEP(uCRC64, pu8); DO_1_STEP(uCRC64, pu8);
#define DO_4_STEP(uCRC64, pu8)  DO_2_STEP(uCRC64, pu8); DO_2_STEP(uCRC64, pu8);
#define DO_8_STEP(uCRC64, pu8)  DO_4_STEP(uCRC64, pu8); DO_4_STEP(uCRC64, pu8);
#define DO_16_STEP(uCRC64, pu8)  DO_8_STEP(uCRC64, pu8); DO_8_STEP(uCRC64, pu8);

#ifdef __x86_64__
#define DO_1_OPTIMIZED_STEP(uCRC64, pu8)  \
  __asm__ __volatile__(   \
                          "movq (%1),%%mm0\n\t"  \
                          "pxor  %0, %%mm0\n\t"   \
                          "pextrw $0, %%mm0, %%eax\n\t"   \
                          "movzbq %%al, %%r8\n\t" \
                          "movzwl (%2,%%r8,2), %%ecx\n\t"        \
                          "pinsrw $0, %%ecx, %%mm3\n\t"      \
                          "pextrw $3, %%mm0, %%ebx\n\t"   \
                          "xorb %%bh, %%cl\n\t" \
                          "movzbq %%cl, %%r8\n\t" \
                          "movzwl (%2,%%r8,2), %%edx\n\t" \
                          "pinsrw $3, %%edx, %%mm4\n\t"    \
                          "movb %%ah, %%al\n\t"   \
                          "movzbq %%al, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%ecx\n\t"    \
                          "pinsrw $0, %%ecx, %%mm4\n\t"    \
                          "movzbq %%bl, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%edx\n\t"       \
                          "pinsrw $3, %%edx, %%mm3\n\t"    \
                          "pextrw $1, %%mm0, %%eax\n\t"   \
                          "movzbq %%al, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%ecx\n\t" \
                          "pinsrw $1, %%ecx, %%mm3\n\t"    \
                          "movb %%ah, %%al\n\t"   \
                          "movzbq %%al, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%edx\n\t" \
                          "pinsrw $1, %%edx, %%mm4\n\t"    \
                          "pextrw $2, %%mm0, %%ebx\n\t"   \
                          "movzbq %%bl, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%ecx\n\t" \
                          "pinsrw $2, %%ecx, %%mm3\n\t"    \
                          "movb %%bh, %%al\n\t"   \
                          "movzbq %%al, %%r8\n\t" \
                          "movzwl (%2, %%r8,2), %%edx\n\t" \
                          "pinsrw $2, %%edx, %%mm4\n\t"    \
                          "psrlq $8, %%mm3\n\t" \
                          "pxor %%mm3, %%mm4\n\t" \
                          "movq %%mm4, %0\n\t"    \
                          :"+&y" (uCRC64) \
                          :"r"(pu8),"D"(s_optimized_crc64_table)   \
                          :"eax","ebx","ecx","edx","mm0","mm3","mm4","r8");
#else
#define DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0)  \
  u_data = uCRC64 ^ (*(uint64_t*)pu8); \
  table_result0 = s_optimized_crc64_table[(uint8_t)u_data];    \
  operand1 = ((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 48)] << 40)     \
             |((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 32)] << 24)   \
             | ((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 16)] << 8)   \
             |( (uint64_t)table_result0 >> 8 );   \
  operand2 = ((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 56) ^ ((uint8_t)table_result0)] << 48)   \
             |((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 40)] << 32)   \
             | ((uint64_t)s_optimized_crc64_table[(uint8_t)(u_data >> 24)] << 16)  \
             |(s_optimized_crc64_table[(uint8_t)(u_data >> 8)]);  \
  uCRC64 = operand1 ^ operand2;
#endif
/**
  * Processes a multiblock of a CRC64 calculation.
  *
  * @returns Intermediate CRC64 value.
  * @param   uCRC64  Current CRC64 intermediate value.
  * @param   pv      The data block to process.
  * @param   cb      The size of the data block in bytes.
  */
uint64_t s3_crc64_optimized(uint64_t uCRC64, const void *pv, int64_t cb)
{
  const uint8_t *pu8 = (const uint8_t *)pv;
#ifndef __x86_64__
  uint64_t u_data = 0, operand1 = 0, operand2 = 0;
  uint16_t table_result0 = 0;
#endif

  if (pv != NULL && cb != 0) {
    while (cb >= 64) {
#ifdef __x86_64__
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8);
      pu8 += 8;

#else
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
      DO_1_OPTIMIZED_STEP(uCRC64, pu8, u_data, operand1, operand2, table_result0);
      pu8 += 8;
#endif
      cb -= 64;
    }
#ifdef __x86_64__
    __asm__ __volatile__(
        "emms"
    );
#endif
    while (cb--) {
      DO_1_STEP(uCRC64, pu8);
    }
  }

  return uCRC64;
}

/**
  * Processes a multiblock of a CRC64 calculation.
  *
  * @returns Intermediate CRC64 value.
  * @param   uCRC64  Current CRC64 intermediate value.
  * @param   pv      The data block to process.
  * @param   cb      The size of the data block in bytes.
  */
uint64_t s3_crc64_again(uint64_t uCRC64, const void *pv, int64_t cb)
{
  return s3_crc64_sse42_again(uCRC64, pv, cb);
}

/**
  * Calculate CRC64 for a memory block.
  *
  * @returns CRC64 for the memory block.
  * @param   pv      Pointer to the memory block.
  * @param   cb      Size of the memory block in bytes.
  */
uint64_t s3_crc64(const void *pv, int64_t cb)
{
  uint64_t  uCRC64 = 0ULL;
  return s3_crc64_again(uCRC64, pv, cb);
}

/**
  * Get the static CRC64 table. This function is only used for testing purpose.
  *
  */
const uint64_t *s3_get_crc64_table()
{
  return s_crc64_table;
}

/**
 * CRC32 implementation from Facebook, based on the zlib
 * implementation.
 *
 * The below CRC32 implementation is based on the implementation
 * included with zlib with modifications to process 8 bytes at a
 * time and using SSE 4.2 extentions when available.  The
 * polynomial constant has been changed to match the one used by
 * SSE 4.2 and does not return the same value as the version
 * used by zlib.  This implementation only supports 64-bit
 * little-endian processors.  The original zlib copyright notice
 * follows.
 *
 * crc32.c -- compute the CRC-32 of a buf stream Copyright (C)
 * 1995-2005 Mark Adler For conditions of distribution and use,
 * see copyright notice in zlib.h
 *
 * Thanks to Rodney Brown <rbrown64@csc.com.au> for his
 * contribution of faster CRC methods: exclusive-oring 32 bits
 * of buf at a time, and pre-computing tables for updating the
 * shift register in one step with three exclusive-ors instead
 * of four steps with four exclusive-ors.  This results in about
 * a factor of two increase in speed on a Power PC G4 (PPC7455)
 * using gcc -O3.
 *
 * this code copy from:
 * http://bazaar.launchpad.net/~mysql/mysql-server/5.6/view/head:/storage/innobase/ut/ut0crc32.cc
 */
#if defined(__GNUC__) && defined(__x86_64__)
/* opcodes taken from objdump of "crc32b (%%rdx), %%rcx"
for RHEL4 support (GCC 3 doesn't support this instruction) */
#define crc32_sse42_byte \
  asm(".byte 0xf2, 0x48, 0x0f, 0x38, 0xf0, 0x0a" \
      : "=c"(crc) : "c"(crc), "d"(buf)); \
  len--, buf++

/* opcodes taken from objdump of "crc32q (%%rdx), %%rcx"
for RHEL4 support (GCC 3 doesn't support this instruction) */
#define crc32_sse42_quadword \
  asm(".byte 0xf2, 0x48, 0x0f, 0x38, 0xf1, 0x0a" \
      : "=c"(crc) : "c"(crc), "d"(buf)); \
  len -= 8, buf += 8
#endif /* defined(__GNUC__) && defined(__x86_64__) */

inline static uint64_t crc64_sse42(uint64_t uCRC64,
                                   const char *buf, int64_t len)
{
  uint64_t crc = uCRC64;

  if (NULL != buf && len > 0) {
    while (len && ((uint64_t) buf & 7)) {
      crc32_sse42_byte;
    }

    while (len >= 32) {
      crc32_sse42_quadword;
      crc32_sse42_quadword;
      crc32_sse42_quadword;
      crc32_sse42_quadword;
    }

    while (len >= 8) {
      crc32_sse42_quadword;
    }

    while (len) {
      crc32_sse42_byte;
    }
  }

  return crc;
}

static uint64_t crc64_sse42_manually(uint64_t crc, const char *buf, int64_t len)
{
  /**
   * crc32tab is generated by:
   *   // bit-reversed poly 0x1EDC6F41
   *   const uint32_t poly = 0x82f63b78;
   *   for (int n = 0; n < 256; n++) {
   *       uint32_t c = (uint32_t)n;
   *       for (int k = 0; k < 8; k++)
   *           c = c & 1 ? poly ^ (c >> 1) : c >> 1;
   *       crc32tab[n] = c;
   *   }
   */
  const static uint32_t crc32tab[] =
  {
    0x00000000L, 0xf26b8303L, 0xe13b70f7L, 0x1350f3f4L, 0xc79a971fL,
    0x35f1141cL, 0x26a1e7e8L, 0xd4ca64ebL, 0x8ad958cfL, 0x78b2dbccL,
    0x6be22838L, 0x9989ab3bL, 0x4d43cfd0L, 0xbf284cd3L, 0xac78bf27L,
    0x5e133c24L, 0x105ec76fL, 0xe235446cL, 0xf165b798L, 0x030e349bL,
    0xd7c45070L, 0x25afd373L, 0x36ff2087L, 0xc494a384L, 0x9a879fa0L,
    0x68ec1ca3L, 0x7bbcef57L, 0x89d76c54L, 0x5d1d08bfL, 0xaf768bbcL,
    0xbc267848L, 0x4e4dfb4bL, 0x20bd8edeL, 0xd2d60dddL, 0xc186fe29L,
    0x33ed7d2aL, 0xe72719c1L, 0x154c9ac2L, 0x061c6936L, 0xf477ea35L,
    0xaa64d611L, 0x580f5512L, 0x4b5fa6e6L, 0xb93425e5L, 0x6dfe410eL,
    0x9f95c20dL, 0x8cc531f9L, 0x7eaeb2faL, 0x30e349b1L, 0xc288cab2L,
    0xd1d83946L, 0x23b3ba45L, 0xf779deaeL, 0x05125dadL, 0x1642ae59L,
    0xe4292d5aL, 0xba3a117eL, 0x4851927dL, 0x5b016189L, 0xa96ae28aL,
    0x7da08661L, 0x8fcb0562L, 0x9c9bf696L, 0x6ef07595L, 0x417b1dbcL,
    0xb3109ebfL, 0xa0406d4bL, 0x522bee48L, 0x86e18aa3L, 0x748a09a0L,
    0x67dafa54L, 0x95b17957L, 0xcba24573L, 0x39c9c670L, 0x2a993584L,
    0xd8f2b687L, 0x0c38d26cL, 0xfe53516fL, 0xed03a29bL, 0x1f682198L,
    0x5125dad3L, 0xa34e59d0L, 0xb01eaa24L, 0x42752927L, 0x96bf4dccL,
    0x64d4cecfL, 0x77843d3bL, 0x85efbe38L, 0xdbfc821cL, 0x2997011fL,
    0x3ac7f2ebL, 0xc8ac71e8L, 0x1c661503L, 0xee0d9600L, 0xfd5d65f4L,
    0x0f36e6f7L, 0x61c69362L, 0x93ad1061L, 0x80fde395L, 0x72966096L,
    0xa65c047dL, 0x5437877eL, 0x4767748aL, 0xb50cf789L, 0xeb1fcbadL,
    0x197448aeL, 0x0a24bb5aL, 0xf84f3859L, 0x2c855cb2L, 0xdeeedfb1L,
    0xcdbe2c45L, 0x3fd5af46L, 0x7198540dL, 0x83f3d70eL, 0x90a324faL,
    0x62c8a7f9L, 0xb602c312L, 0x44694011L, 0x5739b3e5L, 0xa55230e6L,
    0xfb410cc2L, 0x092a8fc1L, 0x1a7a7c35L, 0xe811ff36L, 0x3cdb9bddL,
    0xceb018deL, 0xdde0eb2aL, 0x2f8b6829L, 0x82f63b78L, 0x709db87bL,
    0x63cd4b8fL, 0x91a6c88cL, 0x456cac67L, 0xb7072f64L, 0xa457dc90L,
    0x563c5f93L, 0x082f63b7L, 0xfa44e0b4L, 0xe9141340L, 0x1b7f9043L,
    0xcfb5f4a8L, 0x3dde77abL, 0x2e8e845fL, 0xdce5075cL, 0x92a8fc17L,
    0x60c37f14L, 0x73938ce0L, 0x81f80fe3L, 0x55326b08L, 0xa759e80bL,
    0xb4091bffL, 0x466298fcL, 0x1871a4d8L, 0xea1a27dbL, 0xf94ad42fL,
    0x0b21572cL, 0xdfeb33c7L, 0x2d80b0c4L, 0x3ed04330L, 0xccbbc033L,
    0xa24bb5a6L, 0x502036a5L, 0x4370c551L, 0xb11b4652L, 0x65d122b9L,
    0x97baa1baL, 0x84ea524eL, 0x7681d14dL, 0x2892ed69L, 0xdaf96e6aL,
    0xc9a99d9eL, 0x3bc21e9dL, 0xef087a76L, 0x1d63f975L, 0x0e330a81L,
    0xfc588982L, 0xb21572c9L, 0x407ef1caL, 0x532e023eL, 0xa145813dL,
    0x758fe5d6L, 0x87e466d5L, 0x94b49521L, 0x66df1622L, 0x38cc2a06L,
    0xcaa7a905L, 0xd9f75af1L, 0x2b9cd9f2L, 0xff56bd19L, 0x0d3d3e1aL,
    0x1e6dcdeeL, 0xec064eedL, 0xc38d26c4L, 0x31e6a5c7L, 0x22b65633L,
    0xd0ddd530L, 0x0417b1dbL, 0xf67c32d8L, 0xe52cc12cL, 0x1747422fL,
    0x49547e0bL, 0xbb3ffd08L, 0xa86f0efcL, 0x5a048dffL, 0x8ecee914L,
    0x7ca56a17L, 0x6ff599e3L, 0x9d9e1ae0L, 0xd3d3e1abL, 0x21b862a8L,
    0x32e8915cL, 0xc083125fL, 0x144976b4L, 0xe622f5b7L, 0xf5720643L,
    0x07198540L, 0x590ab964L, 0xab613a67L, 0xb831c993L, 0x4a5a4a90L,
    0x9e902e7bL, 0x6cfbad78L, 0x7fab5e8cL, 0x8dc0dd8fL, 0xe330a81aL,
    0x115b2b19L, 0x020bd8edL, 0xf0605beeL, 0x24aa3f05L, 0xd6c1bc06L,
    0xc5914ff2L, 0x37faccf1L, 0x69e9f0d5L, 0x9b8273d6L, 0x88d28022L,
    0x7ab90321L, 0xae7367caL, 0x5c18e4c9L, 0x4f48173dL, 0xbd23943eL,
    0xf36e6f75L, 0x0105ec76L, 0x12551f82L, 0xe03e9c81L, 0x34f4f86aL,
    0xc69f7b69L, 0xd5cf889dL, 0x27a40b9eL, 0x79b737baL, 0x8bdcb4b9L,
    0x988c474dL, 0x6ae7c44eL, 0xbe2da0a5L, 0x4c4623a6L, 0x5f16d052L,
    0xad7d5351L
  };

  for (int64_t i = 0; i < len; ++i) {
    crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
  }

  return crc;
}

uint64_t crc64_sse42_dispatch(uint64_t crc, const char *buf, int64_t len)
{
  uint32_t a = 0;
  uint32_t b = 0;
  uint32_t c = 0;
  uint32_t d = 0;
  asm("cpuid": "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(1));
  if (c & (1 << 20)) {
    s3_crc64_sse42_func = &crc64_sse42;
  } else {
    s3_crc64_sse42_func = &crc64_sse42_manually;
  }
  return (*s3_crc64_sse42_func)(crc, buf, len);
}

S3CRC64Func s3_crc64_sse42_func = &crc64_sse42_dispatch;
