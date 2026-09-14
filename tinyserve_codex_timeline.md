# TinyServe Codex Implementation Timeline

This file turns `tinyserve_ms_roadmap.md` into an execution timeline for Codex. The goal is to build TinyServe as a serious MS-application project, with Georgia Tech and other AI/CS systems programs in mind.

This is not a fake commit calendar. It is a practical implementation timeline. Codex should move phase by phase, commit only when a milestone is complete, and stop for human review at gates involving model downloads, CUDA setup, benchmark claims, or major scope changes.

## Project Objective

Build:

```text
TinyServe: a minimal C++/CUDA LLM inference runtime for studying transformer decoding, KV caching, quantization, and GPU bottlenecks.
```

The project should prove:

- correctness discipline through reference-logit validation
- ML systems depth through C++/CUDA implementation
- benchmarking maturity through reproducible latency/memory measurements
- honest analysis through documented bottlenecks and limitations

## Timeline Summary

Assume Codex works in focused implementation sessions, with human review at each gate.

| Stage | Estimated Time | Outcome | Resume Value |
| --- | ---: | --- | --- |
| Phase 0 | 1-2 sessions | Repo skeleton, scope docs, benchmark protocol | Shows planning and technical framing |
| Phase 1 | 4-6 sessions | Correct CPU inference baseline | Core "from first principles" evidence |
| Phase 2 | 2-3 sessions | Reference correctness harness | Strong engineering credibility |
| Phase 3 | 3-4 sessions | KV cache + benchmarks | Best LLM systems milestone |
| Phase 4 | 4-6 sessions | CUDA microkernels + profiling notes | Low-level systems depth |
| Phase 5 | 2-4 sessions | INT8 quantization + tradeoff report | Memory/latency analysis |
| Phase 6 | 2-3 sessions | Minimal HTTP serving + load test | Practical serving relevance |
| Phase 7 | 2-3 sessions | Final benchmark report + README polish | Admissions-ready artifact |
| Phase 8 | ongoing | Technical writeups | Human/authentic process evidence |

Minimum strong version:

```text
Phase 0 + Phase 1 + Phase 2 + Phase 3 + Phase 7 + at least one writeup
```

Excellent version:

```text
Phase 0 through Phase 7, plus 2-3 technical writeups
```

## Global Codex Rules

Before every session:

```bash
git status --short
```

Codex must:

1. Preserve user changes.
2. Avoid destructive Git commands.
3. Commit only when the phase gate passes.
4. Keep benchmark numbers script-generated, not manually invented.
5. Keep README claims synchronized with benchmark outputs.
6. Prefer a correct simple implementation over an unfinished optimized one.
7. Stop and ask before GPU-specific setup, model downloads, large files, or benchmark comparisons requiring external tools.

## Phase 0: Repository Skeleton And Scope

Estimated time: 1-2 Codex sessions.

Codex tasks:

1. Create repository structure:

   ```text
   tinyserve/
     README.md
     CMakeLists.txt
     docs/
     runtime/
     tools/
     tests/
     bench/
   ```

2. Add docs:

   ```text
   docs/scope.md
   docs/model_choice.md
   docs/transformer_inference_notes.md
   docs/benchmark_protocol.md
   ```

3. Add a minimal CMake target and test target.
4. Add `.gitignore`.
5. Add first README with honest positioning.

Human review gate:

- confirm model choice
- confirm local hardware target
- confirm whether implementation should start in Windows native CUDA or WSL2/Linux

Acceptance criteria:

- repo builds a placeholder binary
- README says educational/study runtime, not production engine
- docs clearly define allowed and disallowed dependencies

Commit when complete:

```bash
git add README.md CMakeLists.txt .gitignore docs runtime tools tests bench
git commit -m "Add TinyServe scope and inference design skeleton"
```

## Phase 1: Correct CPU Inference Baseline

Estimated time: 4-6 Codex sessions.

Codex tasks:

1. Implement model config parsing:

   ```text
   runtime/model_config.hpp
   runtime/model_config.cpp
   ```

2. Implement weight loading for a simplified converted format:

   ```text
   runtime/weight_loader.hpp
   runtime/weight_loader.cpp
   tools/convert_weights.py
   docs/weights.md
   ```

3. Implement CPU tensor utilities:

   ```text
   runtime/tensor.hpp
   runtime/tensor.cpp
   ```

4. Implement CPU layers:

   ```text
   runtime/layers.hpp
   runtime/layers.cpp
   runtime/attention.hpp
   runtime/attention.cpp
   runtime/sampling.hpp
   runtime/sampling.cpp
   ```

5. Add CLI:

   ```text
   runtime/generate.cpp
   ```

Target command:

```bash
tinyserve generate --model ./models/qwen-0.5b.tserve --prompt "Explain attention" --max-tokens 64
```

Human review gate:

- approve using a tokenizer adapter temporarily if full C++ tokenizer becomes a rabbit hole
- approve model file storage strategy

Acceptance criteria:

- single-sequence greedy decode runs end to end
- output is deterministic for fixed seed / greedy mode
- no PyTorch/HF is used in the runtime path
- README documents current limitations

Suggested commits:

```bash
git commit -m "Add model config and weight loading path"
git commit -m "Implement CPU transformer layers for single-sequence inference"
git commit -m "Add greedy decoding CLI"
```

Commit timing:

- commit loader after converted dummy/model fixture loads successfully
- commit CPU layers after shape/unit tests pass
- commit CLI after a full generation command works

## Phase 2: Reference Correctness Harness

Estimated time: 2-3 Codex sessions.

Codex tasks:

1. Add offline reference generator:

   ```text
   tools/dump_reference.py
   ```

2. Add reference fixtures for small fixed prompts, or document how to generate them locally.
3. Add tests:

   ```text
   tests/test_rmsnorm.py
   tests/test_rope.py
   tests/test_attention_mask.py
   tests/test_logits.py
   ```

4. Add:

   ```text
   docs/correctness.md
   ```

Human review gate:

- confirm acceptable numerical tolerance
- confirm whether reference dumps should be committed or generated locally

Acceptance criteria:

- correctness tests run with one command
- logits or selected intermediate activations are compared against reference outputs
- tolerances are documented

Commit when complete:

```bash
git add tools tests docs/correctness.md
git commit -m "Add reference harness for transformer correctness checks"
```

## Phase 3: KV Cache

Estimated time: 3-4 Codex sessions.

Status: Complete on 2026-08-17 for the deterministic fixture. Cached/no-cache equivalence, retained benchmark samples, and generated result documentation passed; real-model performance remains unvalidated.

Codex tasks:

1. Implement contiguous KV cache:

   ```text
   runtime/kv_cache.hpp
   runtime/kv_cache.cpp
   ```

2. Integrate cache into decode loop.
3. Add correctness test:

   ```text
   cached greedy output == no-cache greedy output
   ```

4. Add benchmark:

   ```text
   bench/kv_cache_bench.cpp
   bench/results.csv
   docs/kv_cache_results.md
   ```

Acceptance criteria:

- cached and no-cache generation match for fixed prompts
- benchmark reports TTFT, decode tokens/sec, and peak memory if available
- docs explain why KV cache improves decode performance

Suggested commits:

```bash
git commit -m "Implement contiguous KV cache for autoregressive decoding"
git commit -m "Benchmark KV cache latency and memory tradeoffs"
git commit -m "Document KV cache results and bottlenecks"
```

This is the most important milestone for MS applications. If later phases slip, keep this polished.

## Phase 4: CUDA Microkernels

Estimated time: 4-6 Codex sessions.

Status: Hardware-gated deferral on 2026-08-17. The optional CMake gate and conditional device probe are complete, but no CUDA compiler/toolkit/device was available to validate RMSNorm, RoPE, GEMV/cuBLAS, or runtime hooks. CPU-only builds remain green; resume from Prompt 024 on validated hardware.

Codex tasks:

1. Add CUDA build support.
2. Implement small kernels:

   ```text
   cuda/rmsnorm.cu
   cuda/rope.cu
   cuda/gemv.cu
   runtime/cuda_ops.cpp
   ```

3. Add microbenchmarks:

   ```text
   bench/cuda_microbench.cpp
   docs/cuda_notes.md
   ```

4. Compare CPU vs CUDA and naive GEMV vs cuBLAS if available.

Human review gate:

- confirm CUDA toolkit availability
- confirm GPU architecture flag
- approve any dependency on cuBLAS

Acceptance criteria:

- kernels produce numerically close outputs
- microbenchmarks are scripted
- docs explain bottlenecks and speedups honestly

Suggested commits:

```bash
git commit -m "Add CUDA RMSNorm and RoPE kernels"
git commit -m "Add CUDA microbenchmarks for decode operations"
git commit -m "Compare naive GEMV with cuBLAS baseline"
git commit -m "Document CUDA bottlenecks from profiling"
```

Fallback rule:

If CUDA setup blocks progress for more than two sessions, preserve the CPU/KV-cache path and turn CUDA into a documented future extension.

## Phase 5: INT8 Quantization

Estimated time: 2-4 Codex sessions.

Codex tasks:

1. Add offline quantizer:

   ```text
   tools/quantize_weights.py
   ```

2. Add INT8 runtime path:

   ```text
   quantization/int8.hpp
   quantization/int8.cpp
   ```

3. Add benchmark:

   ```text
   bench/quantization_bench.cpp
   docs/quantization_results.md
   ```

Measure:

- memory reduction
- decode tokens/sec
- output drift

Acceptance criteria:

- INT8 path generates coherent text
- quality degradation is documented honestly
- memory reduction is measured

Suggested commits:

```bash
git commit -m "Implement weight-only INT8 quantization"
git commit -m "Benchmark INT8 memory and decode tradeoffs"
git commit -m "Document quantization quality drift"
```

## Phase 6: Minimal Serving Layer

Estimated time: 2-3 Codex sessions.

Codex tasks:

1. Add HTTP generation endpoint:

   ```text
   server/http_server.cpp
   server/request_queue.cpp
   ```

2. Add load test:

   ```text
   bench/server_load_test.py
   ```

Target command:

```bash
curl localhost:8000/generate -d '{"prompt":"hello","max_tokens":64}'
```

Acceptance criteria:

- local HTTP generation works
- basic load test reports p50/p95 latency and tokens/sec
- README clearly says this is minimal serving, not production serving

Commit when complete:

```bash
git add server bench README.md docs
git commit -m "Add minimal HTTP generation server and load test"
```

## Phase 7: Final Benchmark And Report

Estimated time: 2-3 Codex sessions.

Codex tasks:

1. Add reproducible benchmark script:

   ```text
   bench/reproduce.sh
   bench/results.csv
   ```

2. Compare against feasible baselines:

   ```text
   Hugging Face Transformers
   llama.cpp
   vLLM only if local hardware supports it
   ```

3. Add:

   ```text
   docs/final_benchmark_report.md
   ```

4. Polish README.

Acceptance criteria:

- benchmark protocol is repeatable
- numbers in README match `bench/results.csv`
- limitations are explicit
- no unsupported speed claims

Commit when complete:

```bash
git add README.md bench docs
git commit -m "Add reproducible benchmark report against established inference engines"
```

## Phase 8: Technical Writing

Estimated time: ongoing, 1 short writeup after key phases.

Codex can draft, but human review should add personal observations and edits.

Write:

```text
docs/blog_single_token_inference.md
docs/blog_kv_cache.md
docs/blog_cuda_bottlenecks.md
```

Each writeup should include:

- what was expected
- what broke
- what was measured
- what was misunderstood initially
- what would be improved next

Suggested commits:

```bash
git commit -m "Write single-token inference walkthrough"
git commit -m "Write KV cache performance analysis"
git commit -m "Write CUDA bottleneck analysis"
```

## Best Implementation Order

Follow this exact order unless blocked:

1. Phase 0
2. Phase 1
3. Phase 2
4. Phase 3
5. Phase 7 partial README/benchmark cleanup for the minimum strong version
6. Phase 4
7. Phase 5
8. Phase 6
9. Phase 7 final polish
10. Phase 8 writeups throughout

Reason:

```text
A correct CPU runtime + correctness harness + KV cache + benchmark report is already a strong MS project.
CUDA, quantization, and serving make it excellent, but they should not block the core deliverable.
```

## Milestone Labels

Tag meaningful milestones:

```text
v0.1-cpu-baseline
v0.2-correctness-harness
v0.3-kv-cache
v0.4-cuda-microbench
v0.5-int8
v1.0-benchmark-report
```

`v0.3-kv-cache` is the suggested current milestone tag. It has not been created because tagging requires an explicit user request and this workspace is not a Git repository.

## Final Resume Bullets

Minimum strong version:

```text
Built TinyServe, a minimal LLM inference runtime implementing transformer decoding, reference-logit validation, KV-cache generation, and reproducible latency/memory benchmarks for studying LLM serving tradeoffs.
```

Excellent version:

```text
Built TinyServe, a minimal C++/CUDA LLM inference runtime implementing transformer decoding, reference-logit validation, KV-cache decoding, CUDA microkernels, INT8 weight quantization, and reproducible latency/memory benchmarks against established inference engines.
```

## Stop Conditions For Codex

Stop and ask the user before:

- downloading large model weights
- changing hardware/toolchain assumptions
- installing CUDA dependencies
- adding large binary artifacts to Git
- claiming benchmark superiority
- using external engines for comparison
- moving from CPU to CUDA if CPU correctness is not complete

## Profile Impact

For MS applications, this project should communicate:

```text
I can go below APIs and frameworks, build a correct inference path, reason about memory and latency, and report systems tradeoffs honestly.
```

This complements the existing profile:

```text
DocVLM-Fingerprint -> multimodal evaluation and reliability
Deepfake study -> robustness and generalization
Perfecter.ai -> applied multimodal AI systems
TinyServe -> low-level ML systems and inference engineering
```
