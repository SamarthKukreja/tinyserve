# KV Cache Benchmark Results

This file is generated from `bench/results.csv` and `bench/raw_results.csv` by `tools/render_kv_results.py`. Do not hand-edit measured values.

## Fixture result

| Mode | Median TTFT (ms) | Median decode (ms) | Decode tok/s | Persistent KV bytes |
| --- | ---: | ---: | ---: | ---: |
| no-cache | 0.180137 | 2.548351 | 2746.874351 | 0 |
| kv-cache | 0.186669 | 0.360349 | 19425.612392 | 192 |

For this run, cached decode throughput was 7.072x the no-cache fixture throughput. The contiguous cache reserves keys and values in `[layer][position][kv_head][head_dimension]` order. It avoids recomputing prior-token projections and decoder blocks, while trading `192` persistent bytes at the measured capacity for the fixture.

## Run metadata

- Timestamp (UTC): `2026-08-17T11:35:40Z`
- Source revision: `unversioned-workspace` (the workspace is not a Git repository)
- Build/compiler: `Release` / `MSVC 194435211`
- Host: `Windows` / `Intel64 Family 6 Model 154 Stepping 4, GenuineIntel`
- Physical RAM bytes / thread settings: `33986252800` / `single-threaded-scalar`
- Model: `dummy.tserve` (`fnv1a64:446d80c55fbd6f0a`, `float32`)
- Batch/seed/sampling: `1` / `0` / `greedy-temperature-0-top-k-0`
- Prompt/generated/decode tokens: `4` / `8` / `7`
- Warmups/repeats/inner iterations: `3` / `9` / `100`
- Process peak memory: `not-measured`; only exact persistent KV allocation is reported
- Command: `C:\Users\SamarthKukreja\Desktop\ai_agent_2026\masters_2027\masters_2027\tinyserve\build-nmake\tinyserve_bench.exe --model build-nmake\fixtures\dummy.tserve --output bench\results.csv --raw-output bench\raw_results.csv --prompt-tokens 4 --generated-tokens 8 --warmups 3 --repeats 9 --inner-iterations 100 --source-revision unversioned-workspace`
- Summary SHA-256: `5a66cc1f8a78fbe0258f11536a148562d0b4b03ad0eaa6607273c60b3508fd4d`
- Raw samples SHA-256: `ed6df7e3c9ee6be9006d9cf19970353d939e9b965d6b8e3f68bd5f9d6cd856fc`

TTFT includes fixture prompt processing and selection of the first generated token. Decode throughput excludes that first token. Each saved sample is the per-request average of the recorded inner iterations, and the table reports the median across repeats.

## Limits

These numbers describe a one-layer, hidden-size-4 synthetic float32 fixture on one local CPU. They are useful for validating the benchmark and exposing the algorithmic tradeoff, but they are not real-model performance, an external-engine comparison, or evidence of language quality. The cached prefill path is deliberately simple and processes prompt tokens incrementally. Paged caching, batching, eviction, prefix reuse, GPU memory, and process peak-memory instrumentation remain out of scope.
