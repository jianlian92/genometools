/*
** Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>
**  
** See LICENSE file or http://genometools.org/license.html for license details.
** 
*/
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <time.h>
#include <sys/time.h>

#include <libgtcore/bitpackstring.h>
#include <libgtcore/env.h>
#include <libgtcore/ensure.h>

enum {
/*   MAX_RND_NUMS = 10, */
  MAX_RND_NUMS = 10000000,
};

static inline int
icmp(uint32_t a, uint32_t b)
{
  if(a > b)
    return 1;
  else if(a < b)
    return -1;
  else /* if(a == b) */
    return 0;
}

int
bitPackString32_unit_test(Env *env)
{
  bitElem *bitStore = NULL;
  uint32_t *randSrc = NULL; /*< create random ints here for input as bit
                                *  store */
  uint32_t *randCmp = NULL; /*< used for random ints read back */
  size_t i, numRnd;
  bitOffset offsetStart, offset;
  unsigned long seedval;
  int had_err = 0;
  {
    struct timeval seed;
    gettimeofday(&seed, NULL);
    srandom(seedval = seed.tv_sec + seed.tv_usec);
  }
  offset = offsetStart = random()%(sizeof(uint32_t) * CHAR_BIT);
  numRnd = random() % MAX_RND_NUMS + 1;
#ifdef VERBOSE_UNIT_TEST
  fprintf(stderr, "seedval = %lu, offset=%lu, numRnd=%lu\n", seedval,
          (long unsigned)offsetStart, (long unsigned)numRnd);
#endif /* VERBOSE_UNIT_TEST */
  {
    bitOffset numBits = sizeof(uint32_t) * CHAR_BIT * numRnd + offsetStart;
    ensure(had_err, (randSrc = env_ma_malloc(env, sizeof(uint32_t)*numRnd))
           && (bitStore = env_ma_malloc(env, bitElemsAllocSize(numBits)
                                  * sizeof(bitElem)))
           && (randCmp = env_ma_malloc(env, sizeof(uint32_t)*numRnd)));
  }
  if(had_err)
  {
    if(randSrc)
      env_ma_free(randSrc, env);
    if(randCmp)
      env_ma_free(randCmp, env);
    if(bitStore)
      env_ma_free(bitStore, env);
#ifdef VERBOSE_UNIT_TEST
    perror("Storage allocations failed");
#endif /* VERBOSE_UNIT_TEST */
    return had_err;
  }
  /* first test unsigned types */
  for(i = 0; i < numRnd; ++i)
  {
#if 32 > 32 && LONG_BIT < 32
    uint32_t v = randSrc[i] = (uint32_t)random() << 32 | random();
#else /* 32 > 32 && LONG_BIT < 32 */
    uint32_t v = randSrc[i] = random();
#endif /* 32 > 32 && LONG_BIT < 32 */
    int bits = requiredUInt32Bits(v);
    bsStoreUInt32(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    uint32_t v = randSrc[i];
    int bits = requiredUInt32Bits(v);
    uint32_t r = bsGetUInt32(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreUInt32/bsGetUInt32: "
              "Expected %"PRIu32", got %"PRIu32", seed = %lu, i = %lu\n",
              v, r, seedval, (unsigned long)i);
#endif /* VERBOSE_UNIT_TEST */
      env_ma_free(randSrc, env);
      env_ma_free(randCmp, env);
      env_ma_free(bitStore, env);
      return had_err;
    }
    offset += bits;
  }
#ifdef VERBOSE_UNIT_TEST
  fputs("bsStoreUInt32/bsGetUInt32: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    uint32_t v0 = randSrc[0];
    int bits0 = requiredUInt32Bits(v0);
    uint32_t r0;
    offset = offsetStart;
    r0 = bsGetUInt32(bitStore, offset, bits0);
    for(i = 1; i < numRnd; ++i)
    {
      uint32_t v1 = randSrc[i];
      int bits1 = requiredUInt32Bits(v1);
      uint32_t r1 = bsGetUInt32(bitStore, offset + bits0, bits1);
      int result;
      ensure(had_err, r0 == v0 && r1 == v1);
      ensure(had_err, icmp(v0, v1) ==
             (result = bsCompare(bitStore, offset, bits0,
                                 bitStore, offset + bits0, bits1)));
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsCompare: "
                "Expected v0 %s v1, got v0 %s v1,\n for v0=%"PRIu32
                " and v1=%"PRIu32",\n"
                "seed = %lu, i = %lu, bits0=%u, bits1=%u\n",
                (v0 > v1?">":(v0 < v1?"<":"==")),
                (result > 0?">":(result < 0?"<":"==")), v0, v1,
                seedval, (unsigned long)i, bits0, bits1);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += bits0;
      bits0 = bits1;
      v0 = v1;
      r0 = r1;
    }
  }
#ifdef VERBOSE_UNIT_TEST
  fputs("bsCompare: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    unsigned numBits = random()%(sizeof(uint32_t)*CHAR_BIT) + 1;
    uint32_t mask = ~(uint32_t)0;
    if(numBits < 32)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformUInt32Array(bitStore, offset, numBits, numRnd, randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      uint32_t v = randSrc[i] & mask;
      uint32_t r = bsGetUInt32(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformUInt32Array/bsGetUInt32: "
                "Expected %"PRIu32", got %"PRIu32",\n seed = %lu,"
                " i = %lu, bits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += numBits;
    }
#ifdef VERBOSE_UNIT_TEST
    fputs("bsStoreUniformUInt32Array/bsGetUInt32: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformUInt32Array(bitStore, offset = offsetStart,
                               numBits, numRnd, randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      uint32_t v = randSrc[i] & mask;
      uint32_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt32Array/bsGetUniformUInt32Array: "
                "Expected %"PRIu32", got %"PRIu32",\n seed = %lu,"
                " i = %lu, bits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }
    {
      uint32_t v = randSrc[0] & mask;
      uint32_t r;
      bsGetUniformUInt32Array(bitStore, offsetStart,
                            numBits, 1, &r);
      if(r != v)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformUInt32Array/bsGetUniformUInt32Array: "
                "Expected %"PRIu32", got %"PRIu32", seed = %lu,"
                " one value extraction\n",
                v, r, seedval);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }    
#ifdef VERBOSE_UNIT_TEST
    fputs(": bsStoreUniformUInt32Array/bsGetUniformUInt32Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  /* int types */
  for(i = 0; i < numRnd; ++i)
  {
    int32_t v = (int32_t)randSrc[i];
    unsigned bits = requiredInt32Bits(v);
    bsStoreInt32(bitStore, offset, bits, v);
    offset += bits;
  }
  offset = offsetStart;
  for(i = 0; i < numRnd; ++i)
  {
    int32_t v = randSrc[i];
    unsigned bits = requiredInt32Bits(v);
    int32_t r = bsGetInt32(bitStore, offset, bits);
    ensure(had_err, r == v);
    if(had_err)
    {
#ifdef VERBOSE_UNIT_TEST
      fprintf(stderr, "bsStoreInt32/bsGetInt32: "
              "Expected %"PRId32", got %"PRId32",\n"
              "seed = %lu, i = %lu, bits=%u\n",
              v, r, seedval, (unsigned long)i, bits);
#endif /* VERBOSE_UNIT_TEST */
      env_ma_free(randSrc, env);
      env_ma_free(randCmp, env);
      env_ma_free(bitStore, env);
      return had_err;
    }
    offset += bits;
  }
#ifdef VERBOSE_UNIT_TEST
  fputs(": bsStoreInt32/bsGetInt32: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  {
    unsigned numBits = random()%(sizeof(int32_t)*CHAR_BIT) + 1;
    int32_t mask = ~(int32_t)0;
    if(numBits < 32)
      mask = ~(mask << numBits);
    offset = offsetStart;
    bsStoreUniformInt32Array(bitStore, offset, numBits, numRnd,
                                (int32_t *)randSrc);
    for(i = 0; i < numRnd; ++i)
    {
      int32_t m = (int32_t)1 << (numBits - 1);
      int32_t v = (int32_t)((randSrc[i] & mask) ^ m) - m;
      int32_t r = bsGetInt32(bitStore, offset, numBits);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr, "bsStoreUniformInt32Array/bsGetInt32: "
                "Expected %"PRId32", got %"PRId32",\n"
                "seed = %lu, i = %lu, numBits=%u\n",
                v, r, seedval, (unsigned long)i, numBits);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
      offset += numBits;
    }
#ifdef VERBOSE_UNIT_TEST
    fputs("bsStoreUniformInt32Array/bsGetInt32: passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
    bsGetUniformInt32Array(bitStore, offset = offsetStart,
                              numBits, numRnd, (int32_t *)randCmp);
    for(i = 0; i < numRnd; ++i)
    {
      int32_t m = (int32_t)1 << (numBits - 1);
      int32_t v = (int32_t)((randSrc[i] & mask) ^ m) - m;
      int32_t r = randCmp[i];
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt32Array/bsGetUniformInt32Array: "
                "Expected %"PRId32", got %"PRId32
                ", seed = %lu, i = %lu\n",
                v, r, seedval, (unsigned long)i);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }
    {
      int32_t m = (int32_t)1 << (numBits - 1);
      int32_t v = (int32_t)((randSrc[0] & mask) ^ m) - m;
      int32_t r;
      bsGetUniformInt32Array(bitStore, offsetStart,
                                numBits, 1, &r);
      ensure(had_err, r == v);
      if(had_err)
      {
#ifdef VERBOSE_UNIT_TEST
        fprintf(stderr,
                "bsStoreUniformInt32Array/bsGetUniformInt32Array: "
                "Expected %"PRId32", got %"PRId32
                ", seed = %lu, one value extraction\n",
                v, r, seedval);
#endif /* VERBOSE_UNIT_TEST */
        env_ma_free(randSrc, env);
        env_ma_free(randCmp, env);
        env_ma_free(bitStore, env);
        return had_err;
      }
    }    
#ifdef VERBOSE_UNIT_TEST
    fputs(": bsStoreUniformInt32Array/bsGetUniformInt32Array:"
          " passed\n", stderr);
#endif /* VERBOSE_UNIT_TEST */
  }
  env_ma_free(randSrc, env);
  env_ma_free(randCmp, env);
  env_ma_free(bitStore, env);
  return had_err;
}

