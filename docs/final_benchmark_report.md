# TinyServe Minimum Strong Version Report

Status: **Wave 5 minimum strong version complete for the deterministic fixture**.

TinyServe now contains a readable C++17 CPU inference path, offline reference comparisons, deterministic generation, a contiguous single-sequence KV cache, cached/no-cache equivalence tests, and a reproducible benchmark with retained raw samples.

## Architecture

The runtime loads a strict versioned tensor file, embeds byte-fixture token IDs, applies scalar RMSNorm, split-half RoPE, grouped-query causal attention, SwiGLU decoder blocks, final normalization, and the LM head. No-cache generation recomputes the full prefix. KV-cache generation processes one new token per step and stores each layer's rotated keys and values contiguously.

## Measured fixture comparison

| Mode | Median TTFT (ms) | Median decode (ms) | Decode tok/s | Persistent KV bytes |
| --- | ---: | ---: | ---: | ---: |
| no-cache | 0.180137 | 2.548351 | 2746.874351 | 0 |
| kv-cache | 0.186669 | 0.360349 | 19425.612392 | 192 |

For this run, cached decode throughput was 7.072x the no-cache fixture throughput. See `docs/kv_cache_results.md` and the checked-in CSV files for the exact environment, command, checksums, and raw repetitions.

## Correctness evidence

- Independent Python fixture references cover primitives, layer output, final hidden state, and full last-token logits.
- Cached incremental logits match the no-cache path within the fixture tolerance.
- Greedy cached and no-cache token sequences match exactly.
- Cache allocation, append/retrieve, capacity, geometry, reset, and repeated-run behavior are tested.

## Honest limits and next steps

Only the synthetic float32 fixture is supported and measured. A real model/tokenizer has not been approved or validated. CUDA, INT8, paged caching, batching, HTTP serving, external-engine comparisons, and production claims are absent. Optional CUDA investigation is the next planned wave and must remain CPU-build-gated.

Suggested milestone tag: `v0.3-kv-cache`. No Git tag was created.
