# TinyServe Roadmap For MS Applications

This roadmap is for building TinyServe as a high-impact MS application project, especially for programs such as Georgia Tech MSCS/MCS and other AI/CS systems-oriented master's programs.

The goal is not to build a production replacement for vLLM, llama.cpp, or Hugging Face Transformers. The goal is to show that I can understand and implement the core systems behind LLM inference, validate correctness, measure tradeoffs, and explain bottlenecks honestly.

## Final Positioning

Use this framing:

```text
TinyServe is a minimal C++/CUDA LLM inference runtime built to study transformer decoding, KV caching, quantization, and GPU bottlenecks. It prioritizes correctness, measurement, and reproducibility over production-level speed.
```

Avoid this framing:

```text
TinyServe is a faster vLLM clone.
TinyServe is production-ready.
TinyServe is state-of-the-art.
```

## Why This Project Helps The Profile

My current profile already has strong applied AI and multimodal evaluation signals:

- DocVLM-Fingerprint: document VLM faithfulness, perturbation robustness, evaluation systems.
- Deepfake generalization study: cross-dataset robustness and video-level evaluation.
- Perfecter.ai work: multimodal AI, document intelligence, visual diffing, and video analytics.

TinyServe adds the missing dimension:

```text
ML systems + low-level inference engineering + CUDA/GPU reasoning.
```

This makes the overall profile stronger because it connects AI application work with the systems layer that makes modern AI practical.

## Core Evidence The Project Must Show

The repository should prove four things:

1. I understand transformer inference below the framework level.
2. I can build correctness tests against trusted reference outputs.
3. I can measure latency, memory, and throughput under a reproducible protocol.
4. I can explain where my implementation loses to production systems and why.

## Scope Rules

Allowed in the inference path:

- C++
- CUDA
- custom runtime code
- custom weight loading format
- custom sampling / decoding code

Allowed only for offline tooling:

- PyTorch
- Hugging Face Transformers
- safetensors conversion
- tokenizer/reference output generation

Not allowed in the TinyServe inference path:

- `transformers.generate`
- PyTorch model inference
- vLLM as a backend
- llama.cpp as a backend

External engines may be used only for comparison benchmarks.

## Recommended Model Choice

Primary:

```text
Qwen2.5-0.5B or Qwen2.5-1.5B
```

Secondary validation:

```text
Llama-3.2-1B
```

Use the smaller model first. A correct, benchmarked 0.5B/1B runtime is far stronger than an unfinished 7B runtime.

## Final Repository Shape

```text
tinyserve/
  README.md
  CMakeLists.txt
  docs/
    scope.md
    model_choice.md
    transformer_inference_notes.md
    correctness.md
    kv_cache_notes.md
    kv_cache_results.md
    cuda_notes.md
    quantization_results.md
    benchmark_protocol.md
    final_benchmark_report.md
    blog_single_token_inference.md
    blog_kv_cache.md
    blog_cuda_bottlenecks.md
  runtime/
    tensor.hpp
    model_config.hpp
    weight_loader.cpp
    layers.cpp
    attention.cpp
    kv_cache.cpp
    sampling.cpp
    generate.cpp
  cuda/
    rmsnorm.cu
    rope.cu
    gemv.cu
  quantization/
    int8.cpp
  server/
    http_server.cpp
    request_queue.cpp
  tools/
    dump_reference.py
    convert_weights.py
    quantize_weights.py
  tests/
    test_tokenizer.py
    test_rmsnorm.py
    test_rope.py
    test_attention_mask.py
    test_logits.py
  bench/
    kv_cache_bench.cpp
    cuda_microbench.cpp
    quantization_bench.cpp
    server_load_test.py
    reproduce.sh
    results.csv
```

## Phase 0: Scope, Environment, And Research Notes

Duration: 3-5 days.

Goal: make the project start from understanding, not from a suspicious code dump.

Create:

```text
docs/scope.md
docs/model_choice.md
docs/transformer_inference_notes.md
docs/kv_cache_notes.md
docs/benchmark_protocol.md
```

Document:

- GPU model, VRAM, driver, CUDA version, compute capability.
- Why this model was chosen.
- What TinyServe will implement.
- What TinyServe will not implement.
- What is allowed only for reference checking.
- Metrics: time to first token, decode tokens/sec, prefill latency, peak memory, correctness error.

Commit when:

- scope is clear
- model choice is justified
- benchmark protocol is written
- repo builds at least a skeleton target

Commit message:

```text
Add project scope and inference design notes
```

## Phase 1: Correct CPU Runtime

Duration: 10-14 days.

Goal: generate tokens correctly without relying on PyTorch or Hugging Face in the inference path.

Implement:

- weight/config loading
- tokenizer path or temporary tokenizer adapter
- embedding lookup
- RMSNorm
- RoPE
- attention
- MLP / SwiGLU
- residual connections
- logits
- greedy decoding
- basic sampling

Deliverable:

```bash
tinyserve generate --model qwen-0.5b --prompt "Explain attention"
```

Commit sequence:

```text
Add weight loader and model config parser
Implement CPU transformer layers for single-sequence inference
Add greedy decoding CLI
Validate CPU logits against reference outputs
```

Acceptance criteria:

- fixed prompts generate stable outputs
- tensor shapes are checked
- logits are compared against reference outputs within a documented tolerance
- README explains this is a correctness-first baseline

## Phase 2: Correctness Harness

Duration: 5-7 days.

Goal: create the most important credibility layer of the project.

Create:

```text
tools/dump_reference.py
tests/test_tokenizer.py
tests/test_rmsnorm.py
tests/test_rope.py
tests/test_attention_mask.py
tests/test_logits.py
docs/correctness.md
```

Correctness strategy:

```text
Use Hugging Face offline to dump reference activations/logits for fixed prompts.
Compare TinyServe intermediate outputs and final logits against those dumps.
```

Commit when:

- reference dumps are reproducible
- tests run from a clean command
- known tolerances are documented

Commit message:

```text
Add reference harness for transformer correctness checks
```

## Phase 3: KV Cache

Duration: 7-10 days.

Goal: implement the first major LLM systems feature.

Implement:

```text
runtime/kv_cache.hpp
runtime/kv_cache.cpp
bench/kv_cache_bench.cpp
docs/kv_cache_results.md
```

Measure:

| Mode | TTFT | Decode tok/s | Peak memory |
| --- | ---: | ---: | ---: |
| No KV cache | TBD | TBD | TBD |
| KV cache | TBD | TBD | TBD |

Commit sequence:

```text
Implement contiguous KV cache for autoregressive decoding
Benchmark KV cache latency and memory tradeoffs
Document KV cache results and bottlenecks
```

Acceptance criteria:

- generation with KV cache matches no-cache greedy output
- benchmark script reports repeatable numbers
- result writeup explains why decode becomes faster

This is the most important milestone. If time gets tight, finish this before moving to CUDA.

## Phase 4: CUDA Acceleration

Duration: 2-3 weeks.

Goal: show GPU systems depth without trying to reimplement every production kernel.

Recommended scope:

```text
CUDA RMSNorm
CUDA RoPE
CUDA GEMV or decode attention microbenchmark
```

Files:

```text
cuda/rmsnorm.cu
cuda/rope.cu
cuda/gemv.cu
runtime/cuda_ops.cpp
bench/cuda_microbench.cpp
docs/cuda_notes.md
```

Benchmarks:

- CPU vs CUDA RMSNorm
- CPU vs CUDA RoPE
- naive GEMV vs cuBLAS if feasible
- memory bandwidth estimate
- kernel launch overhead

Commit sequence:

```text
Add CUDA RMSNorm and RoPE kernels
Add CUDA microbenchmarks for decode operations
Compare naive GEMV with cuBLAS baseline
Document CUDA bottlenecks from profiling
```

Acceptance criteria:

- kernels produce numerically close outputs
- microbenchmarks are scripted
- writeup explains where speedups come from and where they do not

## Phase 5: INT8 Quantization

Duration: 7-10 days.

Goal: show memory-latency-quality tradeoff analysis.

Implement:

```text
tools/quantize_weights.py
quantization/int8.cpp
bench/quantization_bench.cpp
docs/quantization_results.md
```

Start with weight-only INT8:

```text
FP16 weights -> per-channel symmetric INT8 weights + scales -> dequantize during inference
```

Measure:

| Precision | Memory | Decode tok/s | Output drift |
| --- | ---: | ---: | ---: |
| FP16 | TBD | TBD | baseline |
| INT8 | TBD | TBD | TBD |

Commit sequence:

```text
Implement weight-only INT8 quantization
Benchmark INT8 memory and decode tradeoffs
Document quantization quality drift
```

Acceptance criteria:

- INT8 memory savings are measured
- output drift is not hidden
- README does not claim quality is unchanged unless measured

INT4 is optional. Do not attempt INT4 before INT8 is clean.

## Phase 6: Minimal Serving Layer

Duration: 5-7 days.

Goal: connect inference engineering to production-style serving without overbuilding.

Implement:

```text
server/http_server.cpp
server/request_queue.cpp
bench/server_load_test.py
```

Endpoint:

```bash
curl localhost:8000/generate -d '{"prompt":"hello","max_tokens":64}'
```

Measure:

- single-request latency
- concurrent request behavior
- p50 / p95 latency
- aggregate tokens/sec

Commit:

```text
Add minimal HTTP generation server and load test
```

Acceptance criteria:

- local request returns generated text
- load test is scripted
- limitations are documented

## Phase 7: Final Benchmark And Report

Duration: 5-7 days.

Goal: turn the repo into a strong MS-application artifact.

Compare against:

```text
Hugging Face Transformers
llama.cpp
vLLM, only if locally feasible
```

If vLLM does not run on local hardware, document that instead of pretending.

Create:

```text
bench/reproduce.sh
bench/results.csv
docs/final_benchmark_report.md
README.md
```

Report:

- hardware
- model
- precision
- prompt set
- batch size
- TTFT
- decode tokens/sec
- peak memory
- correctness tolerance
- limitations

Commit:

```text
Add reproducible benchmark report against established inference engines
```

Acceptance criteria:

- another person can rerun the benchmark protocol
- results are median-of-3 or otherwise clearly stated
- benchmark claims match `bench/results.csv`
- README and final report agree

## Phase 8: Technical Writing

Write throughout the project, not only at the end.

Recommended docs:

```text
docs/blog_single_token_inference.md
docs/blog_kv_cache.md
docs/blog_cuda_bottlenecks.md
```

Each note should include:

- what I expected
- what broke
- what I measured
- what I misunderstood initially
- what I would improve next

Commit:

```text
Add technical writeups for inference and KV cache design
```

These notes are useful because they show process, not just a final polished artifact.

## Minimum Strong Version

If time is limited, finish only this:

```text
CPU inference
reference correctness harness
KV cache
benchmark suite
technical writeup
```

This is already a strong project for MS applications.

## Excellent Version

The best realistic version:

```text
CPU inference
reference correctness harness
KV cache
CUDA RMSNorm/RoPE/GEMV microbenchmarks
INT8 quantization
minimal HTTP server
final benchmark report
technical writeups
```

Do this only after the minimum strong version is working.

## Stretch Goals

Only attempt these if everything above is stable:

- continuous batching
- simplified paged KV cache
- INT4 quantization
- CUDA graphs
- streaming SSE endpoint
- vLLM benchmark on comparable hardware

Do not let stretch goals block the final deliverable.

## What To Avoid

Avoid claims like:

```text
faster than vLLM
production-ready
state-of-the-art
full inference engine
built from scratch in one weekend
```

Avoid project choices like:

- building a pretty web UI before the runtime works
- adding paged attention before contiguous KV cache is correct
- benchmarking without correctness checks
- changing benchmark numbers manually
- hiding failed experiments

## README Opening

Use something close to:

```text
TinyServe is an educational C++/CUDA LLM inference runtime built to study transformer decoding, KV caching, quantization, and GPU bottlenecks. It prioritizes correctness, measurement, and reproducibility over production-level speed.
```

## Final Resume Bullet

Use this if the excellent version is completed:

```text
Built TinyServe, a minimal C++/CUDA LLM inference runtime implementing transformer decoding, reference-logit validation, KV-cache decoding, CUDA microkernels, INT8 weight quantization, and reproducible latency/memory benchmarks against established inference engines.
```

Use this if only the minimum strong version is completed:

```text
Built TinyServe, a minimal LLM inference runtime implementing transformer decoding, reference-logit validation, KV-cache generation, and reproducible latency/memory benchmarks for studying LLM serving tradeoffs.
```

## Suggested Commit Timeline

This is not a calendar. Commit when the milestone is genuinely complete.

| Milestone | Commit message |
| --- | --- |
| Scope and notes | `Add project scope and inference design notes` |
| Weight loading | `Add weight loader and model config parser` |
| CPU runtime | `Implement CPU transformer layers for single-sequence inference` |
| CLI | `Add greedy decoding CLI` |
| Correctness | `Add reference harness for transformer correctness checks` |
| KV cache | `Implement contiguous KV cache for autoregressive decoding` |
| KV benchmark | `Benchmark KV cache latency and memory tradeoffs` |
| CUDA kernels | `Add CUDA RMSNorm and RoPE kernels` |
| CUDA benchmarks | `Add CUDA microbenchmarks for decode operations` |
| Quantization | `Implement weight-only INT8 quantization` |
| Quantization benchmark | `Benchmark INT8 memory and decode tradeoffs` |
| Server | `Add minimal HTTP generation server and load test` |
| Final benchmark | `Add reproducible benchmark report against established inference engines` |
| Technical writing | `Add technical writeups for inference and KV cache design` |

## Impact On MS Applications

If executed well, TinyServe should strengthen the profile by adding:

- ML systems depth
- C++/CUDA implementation evidence
- benchmarking maturity
- correctness discipline
- honest engineering analysis

For Georgia Tech specifically, this supports the profile theme:

```text
multimodal AI evaluation + applied AI systems + low-level ML inference systems
```

That is a stronger and more coherent profile than adding another generic AI application.
