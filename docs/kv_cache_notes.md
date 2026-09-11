# KV Cache Notes

During autoregressive decoding, keys and values for earlier tokens do not change. A no-cache implementation recomputes them every step; a KV cache retains them per layer so each decode step computes only the new token's key and value and attends over stored history.

TinyServe will first implement a contiguous, single-sequence cache after the no-cache CPU path and correctness harness are working. That order provides a reference path and keeps cache integration testable.

Required validation for the future cache includes:

- Greedy output identical to the no-cache path for fixed inputs
- Shape, capacity, position, reset, and bounds checks
- Clearly separated prefill and decode timing
- Memory accounting tied to layers, heads, head dimension, sequence capacity, and data type
- Repeated measurements under the benchmark protocol

The cache is expected to trade additional persistent memory for less decode recomputation. TinyServe will not claim a throughput improvement until scripts measure it. Paged allocation, continuous batching, eviction, prefix caching, and distributed cache management are out of the initial scope.
