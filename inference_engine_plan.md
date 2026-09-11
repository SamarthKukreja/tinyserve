# Plan: Build "tinyserve" — a minimal LLM inference engine from scratch

**Executor:** AI coding agent (with human review at each milestone gate)
**Stack:** C++/CUDA (chosen over Python+Triton — stronger depth signal for MS applications; raw CUDA is the rarer skill committees notice)
**Hardware:** Local NVIDIA GPU
**Model:** Qwen2.5-1.5B-Instruct (primary), Llama-3.2-1B (secondary/validation)
**Rule:** No PyTorch/HF in the inference path. PyTorch allowed only in offline tools (weight conversion, reference outputs for correctness tests).

---

## Phase 0 — Environment & ground truth (1 session)

0.1 Detect hardware: `nvidia-smi`, record GPU model, VRAM, compute capability, driver/CUDA version. All later kernel decisions (tensor cores, arch flags) depend on this.
0.2 Set up repo: CMake + CUDA toolchain, `tests/`, `bench/`, `tools/`, CI-style `make test` target. Pin compiler and `-arch=sm_XX`.
0.3 Build the **reference harness** (Python, offline only): load Qwen2.5-1.5B via HF transformers, dump per-layer activations and logits for 5 fixed prompts to `.npy` files. This is the correctness oracle for everything that follows.
0.4 Write `tools/convert_weights.py`: HF safetensors → flat binary format (header: config JSON; body: row-major FP16 tensors in a fixed layout). Document the layout in `docs/weights.md`.

**Gate:** repo builds; reference logits dumped; weight file loads back bit-exact in a C++ smoke test.

## Phase 1 — Correct single-sequence inference, FP16 (the "llama2.c in CUDA" milestone)

1.1 Tokenizer: implement BPE/tiktoken-style tokenizer in C++ from the model's `tokenizer.json` (no external libs). Test: round-trip 1,000 strings against HF tokenizer output.
1.2 Kernels, written naively first (correctness before speed): embedding lookup, RMSNorm, GEMM/GEMV (FP16 in, FP32 accumulate), RoPE, softmax, SwiGLU MLP, residual adds, argmax/top-p sampling.
1.3 Attention with a plain contiguous KV cache (one big preallocated tensor per layer).
1.4 End-to-end greedy decode loop. Verify: per-layer max abs error vs reference dumps < 1e-2 (FP16 tolerance); greedy output text matches HF for all 5 fixed prompts to ≥100 tokens.
1.5 Establish baseline numbers: tokens/sec (prefill + decode separately), VRAM usage. Record in `bench/results.md` — this is the "before" column of the final table.

**Gate:** correct greedy generation, benchmarked. *(This alone is the CV-worthy halfway point — commit and tag `v0.1`.)*

## Phase 2 — Make it fast (kernel optimization)

2.1 Profile with Nsight Compute; identify top-3 kernels by time (expect: GEMV, attention).
2.2 Optimized GEMV/GEMM: coalesced loads, vectorized `half2`/`float4` access, shared-memory tiling; tensor-core (WMMA/mma) path if compute capability allows. Target ≥60% of memory bandwidth on decode GEMV (decode is bandwidth-bound — say so in the writeup).
2.3 Fused kernels: RMSNorm+QKV projection, RoPE-in-attention, SwiGLU fusion. Kill kernel-launch overhead with CUDA graphs for the decode step.
2.4 Fused softmax-attention (Flash-attention-style online softmax) for prefill.
2.5 Re-verify correctness after every optimization (same oracle, same tolerance). Re-benchmark; log every change's delta in `bench/results.md`.

**Gate:** ≥3x decode speedup over Phase 1 baseline; correctness still passing.

## Phase 3 — Quantization (INT8, then INT4)

3.1 Offline quantizer in `tools/`: per-channel symmetric INT8 for weights (absmax); keep activations FP16 (W8A16). Extend weight format with scales.
3.2 INT8 GEMV kernel: dequantize-in-register, FP32 accumulate.
3.3 INT4 grouped quantization (group size 64/128, like GPTQ/AWQ storage format — quantize with simple round-to-nearest; note in writeup that GPTQ-style error correction is future work).
3.4 Quality check: perplexity on WikiText-2 (implement PPL eval in the engine) for FP16 vs INT8 vs INT4. Report the degradation honestly.
3.5 Benchmark all three precisions: tokens/sec + VRAM.

**Gate:** INT4 model fits in <2 GB, generates coherent text, PPL delta documented.

## Phase 4 — Serving machinery: paged KV cache + continuous batching

4.1 **Paged KV cache** (the chosen advanced technique): block-based allocator (block = 16 tokens), page table per sequence, block reuse on free. Attention kernel reads through the page table (gather-style addressing).
4.2 Scheduler with **continuous batching**: request queue, admit new sequences into the running batch at each step, evict finished ones; prefill/decode phase separation (chunked prefill optional stretch goal).
4.3 Batched decode kernels: attention and GEMM over ragged batches of sequences at different positions.
4.4 Minimal HTTP server (single-file, e.g. cpp-httplib): OpenAI-compatible `/v1/completions`, streaming via SSE.
4.5 Load-test harness in `bench/`: Poisson request arrivals, measure throughput (tok/s aggregate), p50/p99 time-to-first-token and inter-token latency at batch sizes 1/4/16/32.

**Gate:** 32 concurrent requests served correctly; throughput scales with batch size; no VRAM leak over a 10-min soak.

## Phase 5 — Benchmark showdown + writeup (the deliverable)

5.1 Install llama.cpp and vLLM on the same machine. Convert the same model to their formats (GGUF Q8_0/Q4_K_M; vLLM FP16/INT8).
5.2 Fixed protocol, scripted in `bench/reproduce.sh`: same model, same prompts (ShareGPT sample), same sampling params, 3 runs each, report median. Metrics: prefill tok/s, decode tok/s (batch 1), throughput at batch 16, peak VRAM, TTFT.
5.3 Produce the benchmark table (README front and center) — tinyserve vs llama.cpp vs vLLM at each precision.
5.4 **Honest loss analysis** section: where tinyserve loses and *why* (e.g., no GPTQ error correction, no chunked prefill, weaker GEMM than cuBLAS/Marlin kernels, no CUDA graph capture across batch shapes) — with profiler evidence, not guesses.
5.5 Polish: README with architecture diagram, `reproduce.sh` that goes from clean machine → full table, tag `v1.0`.

**Gate:** a stranger with the same GPU can reproduce the table with one script.

## Phase 6 — Blog posts (interleaved, not deferred)

Write one short post at the end of each of Phases 1, 2, 4, 5:

1. "Building a correct LLM inference loop in raw CUDA" (after P1)
2. "Why decode is memory-bound: profiling and fusing my way to 3x" (after P2)
3. "Implementing paged attention and continuous batching from the vLLM paper" (after P4)
4. "My engine vs llama.cpp vs vLLM: an honest benchmark" (after P5) — this one is the SoP anchor.

---

## Standing rules for the agent

- **Correctness before speed, always.** Every optimization PR must pass the logit-oracle test before benchmarks count.
- **One milestone per branch**; tag on gate pass; `bench/results.md` is append-only history.
- **Verify claims programmatically**: benchmarks are median-of-3, scripted, never hand-run numbers.
- **When stuck >2 sessions on a kernel**, fall back: use a simpler correct version, file the optimization as a known loss for the writeup. A finished honest engine beats an unfinished fast one.
- **Never copy kernel code** from llama.cpp/vLLM. Reading papers and blog posts is fine; the point is implementation from understanding. Reference material: vLLM paper (SOSP '23), Flash-Attention 1/2 papers, karpathy llama2.c, GPTQ/AWQ papers, PMPP book.

## Timeline (≈6 weekends of agent+human sessions)

| Weekend | Milestone |
|---|---|
| 1 | Phase 0 + Phase 1 started |
| 2 | Phase 1 gate (`v0.1`, CV-worthy) |
| 3 | Phase 2 gate |
| 4 | Phase 3 gate |
| 5 | Phase 4 gate |
| 6 | Phase 5 gate (`v1.0`) + final post |

## Risks & mitigations

- **GPU too small for FP16 1.5B + KV cache** (needs ~4–6 GB): use Llama-3.2-1B or quantize earlier (pull Phase 3.1–3.2 before Phase 2).
- **Tokenizer rabbit hole**: if C++ BPE takes >1 session, temporarily tokenize via a Python sidecar and return to it — it's not the depth signal.
- **Windows CUDA toolchain friction**: prefer WSL2 + Linux toolchain if MSVC/CMake fights back.
- **vLLM won't run on small local GPU**: benchmark vLLM on whatever config fits (e.g., INT8 only) and note the asymmetry in the writeup rather than dropping the comparison.
