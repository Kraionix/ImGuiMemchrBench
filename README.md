## Benchmark description

Search for all lines of length `131`, ending with `\n`, in a std::string buffer filled with random ASCII characters. Buffer sizes range from 16 MB to 1 GB. Various memchr implementations using SSE and AVX2 are tested for performance.

## How run

Compiled benchmark in release. Run through `run_benchmark.ps1` to avoid crashes when avx512 is not supported.

## Google benchmark config

Created an experimental json configuration for Google Benchmark using [reflect-cpp](https://github.com/getml/reflect-cpp). Supproted json-schema.

```json
{
  "name": null,
  "function": null,
  "time_unit": "kMillisecond",
  "value_range": {
    "start": 1048576,
    "limit": 1073741824,
    "step": null
  },
  "value_range_list": null,
  "range_multiplier": null,
  "min_time": null,
  "min_warmup_time": null,
  "iterations": null,
  "repetitions": null,
  "report_aggregates_only": null,
  "display_aggregates_only": null,
  "complexity": null,
  "threads": null,
  "thread_range": null
}
```
