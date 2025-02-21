#include <cstdlib>

#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <ranges>
#include <charconv>
#include <random>
#include <memory>
#include <memory_resource>
#include <codecvt>
#include <algorithm>
#include <execution>
#include <span>

#define FMT_STATIC
#define FMT_UNICODE 0
#include <fmt/base.h>
#include <fmt/std.h>
#include <fmt/color.h>

#define BENCHMARK_STATIC_DEFINE
#include <benchmark/benchmark.h>

#include "..\BenchConfigCpp\bench-config.h"

#define IMGUI_ENABLE_AVX2_IMMEMCHR
#include "immemchr.h"


class TestData
{
public:
  TestData(size_t init_size, size_t clip_size = 0, size_t line_size = 0)
    : init_size(init_size), clip_size(clip_size), line_size(line_size), str()
  {
    str.resize(init_size);
    gen_rand_ascii();

    lined_str.resize(init_size - clip_size);
    std::copy(str.data(), str.data() + str.size() - clip_size, lined_str.data());
    set_lines();
  }

  void changeClipSize(size_t clip_size)
  {
    if (clip_size != this->clip_size)
    {
      this->clip_size = clip_size;
      lined_str.resize(init_size - clip_size);
    }
  }

  void changeLineSize(size_t line_size)
  {
    if (line_size != this->line_size)
    {
      this->line_size = line_size;
      std::copy(str.data(), str.data() + str.size() - clip_size, lined_str.data());
      set_lines();
    }
  }

  std::string_view get_str() const
  {
    return lined_str;
  }

  void print() const
  {
    fmt::println("init_size: {}", init_size);
    fmt::println("clip_size: {}", clip_size);
    fmt::println("line_size: {}", line_size);
  }

private:
  void gen_rand_ascii()
  {
    static thread_local std::mt19937 rng(42);
    static thread_local std::uniform_int_distribution<int> dist(32, 126);
    auto l = [&](char& c) { c = static_cast<char>(dist(rng)); };
    std::for_each(std::execution::par_unseq, str.begin(), str.end(), l);
  }

  void set_lines()
  {
    //if (line_size != 0)
    //{
    //  size_t count = lined_str.size() / line_size;
    //  auto view = std::views::iota(size_t(0), count);
    //  auto l = [&](size_t k) { lined_str[k * line_size] = '\n'; };
    //  std::for_each(std::execution::par, view.begin(), view.end(), l);
    //}

    for (size_t i = 0; i < lined_str.size(); i += line_size * 4)
    {
      lined_str[i] = '\n';
      lined_str[i + line_size] = '\n';
      lined_str[i + line_size * 2] = '\n';
      lined_str[i + line_size * 3] = '\n';
    }
  }

private:
  size_t init_size;
  size_t clip_size;
  size_t line_size;

  std::string str;
  std::string lined_str;
};

template <auto MemchrFunc = ImMemchr>
size_t all_lines(const char* buf, size_t size)
{
  const char* ptr = buf;
  const char* end = buf + size;

  while (ptr < end)
  {
    const char* new_line = (const char*)MemchrFunc(ptr, '\n', end - ptr);

    if (new_line)
      ptr = new_line + 1;
    else
      break;
  }

  return ptr - buf;
}


using MemchrFuncT = decltype(ImMemchr);
template <MemchrFuncT MemchrFunc = ImMemchr>
static void BM_AllLines(benchmark::State& state)
{
  size_t size = state.range(0);

  TestData data(size, 0, 131);

  std::string_view strv = data.get_str();
  const char* buf = strv.data();
  size_t buf_size = strv.size();

  for (auto _ : state)
  {
    benchmark::DoNotOptimize(all_lines<MemchrFunc>(buf, buf_size));
  }

  state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}

auto BM_AllLines_AVX512_PREFETCH      = BM_AllLines<ImMemchrAVX512_PREFETCH>;
auto BM_AllLines_AVX512               = BM_AllLines<ImMemchrAVX512>;
auto BM_AllLines_AVX2_UNROLL_PREFETCH = BM_AllLines<ImMemchrAVX2_UNROLL_PREFETCH>;
auto BM_AllLines_AVX2_UNROLL          = BM_AllLines<ImMemchrAVX2_UNROLL>;
auto BM_AllLines_AVX2_PREFETCH        = BM_AllLines<ImMemchrAVX2_PREFETCH>;
auto BM_AllLines_AVX2                 = BM_AllLines<ImMemchrAVX2>;
auto BM_AllLines_SSE_UNROLL_PREFETCH  = BM_AllLines<ImMemchrSSE_UNROLL_PREFETCH>;
auto BM_AllLines_SSE_UNROLL           = BM_AllLines<ImMemchrSSE_UNROLL>;
auto BM_AllLines_SSE_PREFETCH         = BM_AllLines<ImMemchrSSE_PREFETCH>;
auto BM_AllLines_SSE                  = BM_AllLines<ImMemchrSSE>;
auto BM_AllLines_CSTD                 = BM_AllLines<ImMemchrCSTD>;

static fs::path path = fs::current_path() / "bench_config.json";
static benchcfg::ConfigLoader config_loader(path);

BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX512_PREFETCH, "ImMemchr_AVX512_PREFETCH")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX512, "ImMemchr_AVX512")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX2_UNROLL_PREFETCH, "ImMemchr_AVX2_UNROLL_PREFETCH")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX2_UNROLL, "ImMemchr_AVX2_UNROLL")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX2_PREFETCH, "ImMemchr_AVX2_PREFETCH")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_AVX2, "ImMemchr_AVX2")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_SSE_UNROLL_PREFETCH, "ImMemchr_SSE_UNROLL_PREFETCH")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_SSE_UNROLL, "ImMemchr_SSE_UNROLL")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_SSE_PREFETCH,  "ImMemchr_SSE_PREFETCH")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_SSE, "ImMemchr_SSE")
BENCHMARK_FROM_CONFIG_LOADER(config_loader, BM_AllLines_CSTD, "ImMemchr_CSTD")

BENCHMARK_MAIN();