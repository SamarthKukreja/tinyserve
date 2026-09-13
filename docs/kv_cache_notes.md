# KV Cache Notes

During autoregressive decoding, keys and values for earlier tokens do not change. A no-cache implementation recomputes them every step; a KV cache retains them per layer so each decode step computes only the new token's key and value and attends over stored history.

TinyServe implements a contiguous, single-sequence cache after the no-cache CPU path and correctness harness. Keys and values use separate float32 buffers in `[layer][position][kv_head][head_dimension]` order. Each layer has a logical length; a decode step appends exactly one rotated key and value per layer. Reset clears logical lengths so the allocation can be reused.

Wave 5 validation includes:

- Greedy output identical to the no-cache path for fixed inputs
- Shape, capacity, position, reset, and bounds checks
- Clearly separated prefill and decode timing
- Memory accounting tied to layers, heads, head dimension, sequence capacity, and data type
- Repeated measurements under the benchmark protocol

The cache trades additional persistent memory for less decode recomputation. The checked-in fixture measurement and raw samples are documented in [KV cache results](kv_cache_results.md); that narrow measurement must not be generalized to real models. Paged allocation, continuous batching, eviction, prefix caching, and distributed cache management are out of the initial scope.
