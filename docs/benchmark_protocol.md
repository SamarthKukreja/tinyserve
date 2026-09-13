# Benchmark Protocol

This document defines how TinyServe collects performance results. Wave 0's placeholder recorded no measurements; Wave 5 replaces it with the fixture-only no-cache/KV-cache benchmark whose generated summary and retained samples live in `bench/results.csv` and `bench/raw_results.csv`.

## Metrics

- **Time to first token (TTFT):** elapsed wall time from the start of a warmed generation request through production of its first generated token.
- **Prefill latency:** time spent processing the complete input prompt before iterative decode.
- **Decode throughput:** generated decode tokens divided by decode-only elapsed time, reported in tokens per second; the first generated token is excluded when it belongs to the TTFT/prefill measurement.
- **Peak memory:** maximum process memory observed over the measured run, with the platform-specific collection method and host/device scope stated.
- **Correctness tolerance:** the absolute and relative error thresholds used for named tensors or logits, plus exact token agreement for deterministic greedy comparisons where applicable.

## Required run metadata

Every saved result must identify:

- Commit or source revision
- Exact build type and compiler
- Operating system, CPU, RAM, and thread settings
- GPU, VRAM, driver, CUDA version, and compute capability when CUDA is used
- Model identifier and artifact checksum, precision, prompt/token lengths, batch size, seed, and sampling settings
- Command used, warm-up policy, repeat count, and measurement method

The Wave 5 fixture result records its local hardware, RAM, scalar thread setting, compiler, build type, model checksum, command, warmups, and repeats directly from the benchmark harness. Future benchmark paths must capture the same metadata rather than filling it from memory.

## Procedure

1. Build a release configuration and run correctness tests first.
2. Use fixed, versioned prompts and token limits; record token counts rather than relying only on character counts.
3. Warm up explicitly and exclude warm-up runs from the result set.
4. Run at least three measured repetitions for each configuration and retain every raw sample.
5. Report the median as the primary latency/throughput statistic; add spread or percentiles where useful.
6. Change one comparison variable at a time and keep model, prompt, token count, batch size, and hardware fixed.
7. Generate summary CSV/JSON and documentation from saved raw outputs. Do not hand-edit result numbers.

## Correctness before comparison

Performance results are publishable only for paths that have passed their applicable correctness gate. Comparisons against Hugging Face Transformers, llama.cpp, or vLLM must use documented equivalent settings and must identify setup differences. If an engine or hardware path is unavailable, the report will say so.

No speed, memory, or quality comparison is implied by this protocol alone.
