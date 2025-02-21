#pragma once

#include <intrin.h>

#define IMGUI_PREFECTH_LENGTH 1024

const void* ImMemchrAVX512_PREFETCH(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 64;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m512i target = _mm512_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m512i chunk = _mm512_loadu_si512((const __m512i*)ptr);
      uint64_t mask = _mm512_cmpeq_epi8_mask(chunk, target);

      if (mask)
        return (const void*)(ptr + _tzcnt_u64(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m512i chunk = _mm512_load_si512((const __m512i*)ptr);
      uint64_t mask = _mm512_cmpeq_epi8_mask(chunk, target);

      if (mask)
        return (const void*)(ptr + _tzcnt_u64(mask));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrAVX512(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 64;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m512i target = _mm512_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m512i chunk = _mm512_loadu_si512((const __m512i*)ptr);
      uint64_t mask = _mm512_cmpeq_epi8_mask(chunk, target);

      if (mask)
        return (const void*)(ptr + _tzcnt_u64(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m512i chunk = _mm512_load_si512((const __m512i*)ptr);
      uint64_t mask = _mm512_cmpeq_epi8_mask(chunk, target);

      if (mask)
        return (const void*)(ptr + _tzcnt_u64(mask));
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrAVX2_UNROLL_PREFETCH(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 32;
  const size_t SIMD_UNROLLED_LENGTH = 32 * 4;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char* align_unroll_end = end - SIMD_LENGTH * SIMD_UNROLLED_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m256i target = _mm256_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m256i chunk = _mm256_lddqu_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_unroll_end; ptr += SIMD_UNROLLED_LENGTH)
    {
      __m256i chunk1 = _mm256_load_si256((const __m256i*)ptr);
      __m256i chunk2 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH));
      __m256i chunk3 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH * 2));
      __m256i chunk4 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH * 3));

      int mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk1, target));
      int mask2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk2, target));
      int mask3 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk3, target));
      int mask4 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk4, target));

      if (mask1)
        return (const void*)(ptr + _tzcnt_u32(mask1));
      else if (mask2)
        return (const void*)(ptr + SIMD_LENGTH + _tzcnt_u32(mask2));
      else if (mask3)
        return (const void*)(ptr + SIMD_LENGTH * 2 + _tzcnt_u32(mask3));
      else if (mask4)
        return (const void*)(ptr + SIMD_LENGTH * 3 + _tzcnt_u32(mask4));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m256i chunk = _mm256_load_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrAVX2_UNROLL(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 32;
  const size_t SIMD_UNROLLED_LENGTH = 32 * 4;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char* align_unroll_end = end - SIMD_LENGTH * SIMD_UNROLLED_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m256i target = _mm256_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m256i chunk = _mm256_lddqu_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_unroll_end; ptr += SIMD_UNROLLED_LENGTH)
    {
      __m256i chunk1 = _mm256_load_si256((const __m256i*)ptr);
      __m256i chunk2 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH));
      __m256i chunk3 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH * 2));
      __m256i chunk4 = _mm256_load_si256((const __m256i*)(ptr + SIMD_LENGTH * 3));

      int mask1 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk1, target));
      int mask2 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk2, target));
      int mask3 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk3, target));
      int mask4 = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk4, target));

      if (mask1)
        return (const void*)(ptr + _tzcnt_u32(mask1));
      else if (mask2)
        return (const void*)(ptr + SIMD_LENGTH + _tzcnt_u32(mask2));
      else if (mask3)
        return (const void*)(ptr + SIMD_LENGTH * 2 + _tzcnt_u32(mask3));
      else if (mask4)
        return (const void*)(ptr + SIMD_LENGTH * 3 + _tzcnt_u32(mask4));
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m256i chunk = _mm256_load_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrAVX2_PREFETCH(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 32;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m256i target = _mm256_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m256i chunk = _mm256_lddqu_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m256i chunk = _mm256_load_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrAVX2(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 32;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m256i target = _mm256_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m256i chunk = _mm256_lddqu_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)_andn_u64(SIMD_LENGTH_MASK, (uintptr_t)ptr + SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m256i chunk = _mm256_load_si256((const __m256i*)ptr);
      int mask = _mm256_movemask_epi8(_mm256_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrSSE_UNROLL_PREFETCH(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 16;
  const size_t SIMD_UNROLLED_LENGTH = 16 * 4;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char* align_unroll_end = end - SIMD_UNROLLED_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m128i target = _mm_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)(((uintptr_t)ptr + SIMD_LENGTH_MASK) & ~SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_unroll_end; ptr += SIMD_UNROLLED_LENGTH)
    {
      __m128i chunk1 = _mm_load_si128((const __m128i*)ptr);
      __m128i chunk2 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH));
      __m128i chunk3 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH * 2));
      __m128i chunk4 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH * 3));

      int mask1 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk1, target));
      int mask2 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk2, target));
      int mask3 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk3, target));
      int mask4 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk4, target));

      if (mask1)
        return (const void*)(ptr + _tzcnt_u32(mask1));
      else if (mask2)
        return (const void*)(ptr + SIMD_LENGTH + _tzcnt_u32(mask2));
      else if (mask3)
        return (const void*)(ptr + SIMD_LENGTH * 2 + _tzcnt_u32(mask3));
      else if (mask4)
        return (const void*)(ptr + SIMD_LENGTH * 3 + _tzcnt_u32(mask4));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m128i chunk = _mm_load_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrSSE_UNROLL(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 16;
  const size_t SIMD_UNROLLED_LENGTH = 16 * 4;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char* align_unroll_end = end - SIMD_UNROLLED_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end) 
  {
    const __m128i target = _mm_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m128i chunk = _mm_loadu_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)(((uintptr_t)ptr + SIMD_LENGTH_MASK) & ~SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_unroll_end; ptr += SIMD_UNROLLED_LENGTH)
    {
      __m128i chunk1 = _mm_load_si128((const __m128i*)ptr);
      __m128i chunk2 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH));
      __m128i chunk3 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH * 2));
      __m128i chunk4 = _mm_load_si128((const __m128i*)(ptr + SIMD_LENGTH * 3));

      int mask1 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk1, target));
      int mask2 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk2, target));
      int mask3 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk3, target));
      int mask4 = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk4, target));

      if (mask1)
        return (const void*)(ptr + _tzcnt_u32(mask1));
      else if (mask2)
        return (const void*)(ptr + SIMD_LENGTH + _tzcnt_u32(mask2));
      else if (mask3)
        return (const void*)(ptr + SIMD_LENGTH * 2 + _tzcnt_u32(mask3));
      else if (mask4)
        return (const void*)(ptr + SIMD_LENGTH * 3 + _tzcnt_u32(mask4));
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m128i chunk = _mm_load_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));
    }
  }

  for (; ptr < end; ptr++) 
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrSSE_PREFETCH(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 16;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m128i target = _mm_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m128i chunk = _mm_lddqu_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)(((uintptr_t)ptr + SIMD_LENGTH_MASK) & ~SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m128i chunk = _mm_load_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      if (ptr <= end - IMGUI_PREFECTH_LENGTH)
        _mm_prefetch((const char*)(ptr + IMGUI_PREFECTH_LENGTH), _MM_HINT_T0);
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrSSE(const void* buf, int val, size_t count)
{
  const size_t SIMD_LENGTH = 16;
  const size_t SIMD_LENGTH_MASK = SIMD_LENGTH - 1;

  const unsigned char* ptr = (const unsigned char*)buf;
  const unsigned char* end = ptr + count;
  const unsigned char* align_end = end - SIMD_LENGTH;
  const unsigned char ch = (const unsigned char)val;

  if (ptr <= align_end)
  {
    const __m128i target = _mm_set1_epi8(ch);

    if ((uintptr_t)ptr & SIMD_LENGTH_MASK)
    {
      __m128i chunk = _mm_lddqu_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));

      ptr = (const unsigned char*)(((uintptr_t)ptr + SIMD_LENGTH_MASK) & ~SIMD_LENGTH_MASK);
    }

    for (; ptr <= align_end; ptr += SIMD_LENGTH)
    {
      __m128i chunk = _mm_load_si128((const __m128i*)ptr);
      int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(chunk, target));

      if (mask)
        return (const void*)(ptr + _tzcnt_u32(mask));
    }
  }

  for (; ptr < end; ptr++)
  {
    if (*ptr == ch)
      return (const void*)(ptr);
  }

  return nullptr;
}

const void* ImMemchrCSTD(const void* buf, int val, size_t count)
{
  return memchr(buf, val, count);
}


#if defined IMGUI_ENABLE_AVX512_IMMEMCHR
const void* ImMemchr(const void* buf, int val, size_t count)
{
  return ImMemchrAVX512(buf, val, count);
}
#elif defined IMGUI_ENABLE_AVX2_UNROLL_IMMEMCHR
const void* ImMemchr(const void* buf, int val, size_t count)
{
  return ImMemchrAVX2_UNROLL(buf, val, count);
}
#elif defined IMGUI_ENABLE_AVX2_IMMEMCHR
const void* ImMemchr(const void* buf, int val, size_t count)
{
  return ImMemchrAVX2(buf, val, count);
}
#elif defined IMGUI_ENABLE_SSE_IMMEMCHR
const void* ImMemchr(const void* buf, int val, size_t count)
{
  return ImMemchrSSE(buf, val, count);
}
#else
const void* ImMemchr(const void* buf, int val, size_t count)
{
  return ImMemchrCSTD(buf, val, count);
}
#endif