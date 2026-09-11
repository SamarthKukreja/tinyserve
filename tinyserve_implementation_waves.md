# TinyServe Implementation Waves

This file groups the TinyServe Codex prompts into larger implementation waves. Each wave is designed to be a coherent single-pass Codex task: it should produce one stable layer of the project, run the relevant validation, update the progress tracker, and leave the repository in a commit-ready state.

Use this alongside:

- `tinyserve_ms_roadmap.md`
- `tinyserve_codex_timeline.md`
- `tinyserve_codex_execution_prompts.md`

Core rule: do not fabricate progress. If a wave cannot fully complete because of missing hardware, missing model access, failed dependency installation, or runtime constraints, document the blocker clearly and commit the honest state only if the code/docs remain useful.

---

## How To Use These Waves

1. Give Codex one wave at a time.
2. Ask Codex to read the relevant prompts from `tinyserve_codex_execution_prompts.md` before implementing.
3. Require Codex to update `tinyserve_implementation_progress.md` inside every wave.
4. Commit only after the wave validates cleanly or documents an honest deferral.
5. Do not squash all waves into one commit. The point is to create meaningful engineering checkpoints, not artificial daily noise.

Recommended cadence:

- If a wave is small and clean, finish it in one day and commit once.
- If a wave is large, split it at the prompt boundaries already listed in `tinyserve_codex_execution_prompts.md`.
- If training/model conversion/benchmarking takes many hours, commit code before the long run, then commit results after the run finishes.

---

# Wave 0 — Project Foundation And Execution Control

Prompts:

- Prompt 000 — Initialize Progress Tracker
- Prompt 001 — Project Foundation And Repository Skeleton
- Prompt 002 — Scope, Model Choice, And Benchmark Protocol Docs
- Prompt 003 — Developer Tooling, Build Targets, And Baseline Test Harness

Goal:

Create a professional repository foundation before writing inference logic. This wave should make the project understandable to another engineer or admissions reviewer: what is being built, what is intentionally out of scope, how it will be tested, and how progress will be tracked.

Why this is one clean pass:

These prompts are planning, structure, and tooling. They do not depend on model math or GPU hardware, so they can be completed together without waiting for long experiments.

Expected output:

- Repository skeleton
- Initial README
- Project scope document
- Model and benchmark protocol notes
- Build/test command stubs
- Initial `tinyserve_implementation_progress.md`

Validation:

- Build command exists, even if minimal.
- Test command exists, even if only smoke tests.
- README does not claim unsupported performance.
- Progress tracker lists all prompts and phase statuses.

Commit timing:

Commit at the end of the wave after the skeleton and docs are internally consistent.

Suggested commit message:

```bash
git commit -m "Add TinyServe project foundation and execution plan"
```

---

# Wave 1 — Model Boundary, Weights, And Tokenization

Prompts:

- Prompt 004 — Model Config Schema And TinyServe Weight Format
- Prompt 006 — Weight Conversion Tool With Dummy Fixture
- Prompt 007 — C++ Weight Loader And Fixture Loading Tests
- Prompt 008 — Tokenizer Boundary And Temporary Adapter

Goal:

Define the boundary between external model assets and TinyServe's runtime. This wave should make it clear how a model config is represented, how weights are converted into a TinyServe-friendly format, how the C++ runtime loads them, and how tokenization is handled without overclaiming full Hugging Face compatibility.

Why this is one clean pass:

All four prompts define I/O contracts. They can be implemented with deterministic dummy fixtures before real model conversion or heavy inference is needed.

Expected output:

- Model config schema
- TinyServe weight format documentation
- Offline dummy conversion tool
- C++ weight loader
- Fixture model files
- Tokenizer interface or adapter boundary
- Loader and tokenizer tests

Validation:

- Dummy fixture conversion is deterministic.
- C++ loader reads the fixture and validates metadata.
- Tokenizer adapter tests pass.
- Docs clearly separate fixture support from real-model support.

Commit timing:

Commit after fixture conversion, loading, and tokenizer boundary tests pass.

Suggested commit message:

```bash
git commit -m "Add model format, weight loading, and tokenizer boundary"
```

Optional split commits if the wave becomes large:

```bash
git commit -m "Add model config schema and weight format docs"
git commit -m "Add offline weight conversion fixture"
git commit -m "Add TinyServe weight loader and tokenizer adapter"
```

---

# Wave 2 — CPU Transformer Math Core

Prompts:

- Prompt 005 — CPU Tensor Utilities And Numeric Primitives
- Prompt 009 — CPU RMSNorm And RoPE
- Prompt 010 — CPU Attention Masking And Softmax
- Prompt 011 — CPU MLP, SwiGLU, Residuals, And Layer Composition

Goal:

Build the CPU math primitives needed for a decoder-only transformer. This wave should focus on correctness, simple readable implementations, and testable intermediate outputs rather than speed.

Why this is one clean pass:

These prompts are tightly related math kernels. Implementing them together helps avoid mismatched tensor conventions, indexing rules, shape assumptions, and residual-flow bugs.

Expected output:

- CPU tensor utilities
- Shape/index helpers
- RMSNorm implementation
- RoPE implementation
- Causal attention and softmax
- MLP/SwiGLU implementation
- Decoder block composition
- Unit tests for each math component

Validation:

- Unit tests pass for tensor utilities.
- RMSNorm and RoPE match hand-computed or fixture references.
- Causal mask behavior is tested.
- Decoder block tests verify shape and deterministic output.

Commit timing:

Commit after all CPU math component tests pass.

Suggested commit message:

```bash
git commit -m "Implement CPU transformer math core"
```

Optional split commits:

```bash
git commit -m "Add CPU tensor utilities"
git commit -m "Implement CPU RMSNorm and RoPE"
git commit -m "Implement CPU attention and decoder block components"
```

---

# Wave 3 — CPU Forward Pass And Generation CLI

Prompts:

- Prompt 012 — Full CPU Model Forward Pass
- Prompt 013 — Sampling And Greedy Decoding
- Prompt 014 — Generation CLI End-To-End CPU Baseline

Goal:

Turn the math components into an end-to-end CPU inference path. By the end of this wave, TinyServe should run a fixture model through a full forward pass and generate tokens through a CLI.

Why this is one clean pass:

The full model forward pass, decoding loop, and CLI are one natural vertical slice. This wave creates the first visible version of the project.

Expected output:

- Full CPU model forward pass
- Greedy decoding
- Optional top-k/top-p sampling if already scoped
- CLI generation command
- End-to-end fixture smoke test

Validation:

- Fixture model forward pass test passes.
- Greedy decoding is deterministic.
- CLI generates output from a fixture prompt.
- No README claim suggests real-model quality unless already validated.

Commit timing:

Commit after CLI smoke tests and CPU generation tests pass.

Suggested commit message:

```bash
git commit -m "Add end-to-end CPU generation baseline"
```

---

# Wave 4 — Correctness Harness Against References

Prompts:

- Prompt 015 — Offline Reference Dump Tool
- Prompt 016 — Layer-Level Correctness Tests Against References
- Prompt 017 — Correctness Documentation And Real-Model Gate

Goal:

Create the evidence layer for correctness. This wave should make it possible to compare TinyServe outputs against saved reference tensors/logits without requiring internet access or model downloads during normal tests.

Why this is one clean pass:

Reference dumping, reference-based tests, and correctness documentation belong together. The implementation is only credible if the tests and methodology are committed together.

Expected output:

- Offline reference dump tool
- Saved fixture references
- Layer-level comparison tests
- Logit comparison tests where feasible
- Correctness methodology document
- Real-model validation gate with exact commands

Validation:

- Reference tests pass on checked-in fixtures.
- Tolerances are documented.
- Any real-model validation that cannot run locally is marked as gated, not claimed complete.

Commit timing:

Commit after fixture reference tests pass and real-model validation status is documented.

Suggested commit message:

```bash
git commit -m "Add reference correctness harness"
```

---

# Wave 5 — KV Cache And Minimum Strong Version

Prompts:

- Prompt 018 — Baseline No-Cache Decode Benchmark
- Prompt 019 — Contiguous KV Cache Data Structure
- Prompt 020 — Integrate KV Cache Into Decode Loop
- Prompt 021 — KV Cache Benchmark And Results Documentation
- Prompt 022 — Minimum Strong Version Report And README Pass

Goal:

Move from a simple CPU baseline to a serious inference-engine project. This wave should show the core systems idea: measuring no-cache decoding, implementing KV cache, verifying identical outputs, and reporting the latency/memory tradeoff.

Why this is one clean pass:

KV cache is most meaningful when paired with the baseline benchmark and final comparison. This wave should end with the minimum version that is already resume-worthy.

Expected output:

- No-cache decode benchmark
- Contiguous KV cache implementation
- Cached decode path
- Cached-vs-no-cache equivalence tests
- KV latency and memory benchmark
- Minimum strong version README/report update

Validation:

- Cached and no-cache decoding produce matching outputs on fixture prompts.
- Benchmark scripts are reproducible.
- Benchmark result files include machine/environment details.
- README and report numbers match saved outputs.

Commit timing:

Commit before any long benchmark run if the benchmark code is ready. Commit again after benchmark results are complete and documented.

Suggested commit messages:

```bash
git commit -m "Add KV cache decode path"
git commit -m "Benchmark KV cache latency and memory tradeoffs"
git commit -m "Package TinyServe minimum strong version"
```

If completed quickly in one clean pass:

```bash
git commit -m "Add KV cache and minimum strong version report"
```

---

# Wave 6 — Optional CUDA Acceleration Track

Prompts:

- Prompt 023 — CUDA Build Gate And Hardware Detection
- Prompt 024 — CUDA RMSNorm Kernel And Tests
- Prompt 025 — CUDA RoPE Kernel And Tests
- Prompt 026 — CUDA GEMV Microbenchmark And cuBLAS Baseline
- Prompt 027 — CUDA Bottleneck Documentation And Optional Runtime Hooks

Goal:

Add a credible GPU-aware systems layer without letting CUDA become a blocker for the entire project. This wave should either implement small CUDA kernels and microbenchmarks or document a clean hardware-gated deferral.

Why this is one clean pass:

CUDA support should be isolated behind optional build gates. That makes this wave independent from the CPU minimum strong version.

Expected output:

- Optional CUDA build detection
- CPU-only fallback remains intact
- CUDA RMSNorm kernel if hardware/toolchain permits
- CUDA RoPE kernel if hardware/toolchain permits
- GEMV/cuBLAS microbenchmark if feasible
- CUDA bottleneck notes
- Honest runtime hook status

Validation:

- CPU-only build still passes.
- CUDA tests pass when CUDA is available.
- CUDA-unavailable environments skip cleanly.
- Documentation clearly states implemented vs deferred CUDA work.

Commit timing:

Commit after the build gate is stable. Commit kernel work only after correctness tests pass or honest deferral is documented.

Suggested commit messages:

```bash
git commit -m "Add optional CUDA build gate"
git commit -m "Add CUDA RMSNorm and RoPE kernels"
git commit -m "Document CUDA bottlenecks and runtime hooks"
```

If CUDA is unavailable:

```bash
git commit -m "Document CUDA-gated acceleration path"
```

---

# Wave 7 — Quantization And Serving Layer

Prompts:

- Prompt 028 — Weight-Only INT8 Quantization Format And Tool
- Prompt 029 — INT8 Runtime Path
- Prompt 030 — INT8 Benchmark And Quality Drift Report
- Prompt 031 — Minimal HTTP Generation Server
- Prompt 032 — Server Load Test And Latency Reporting

Goal:

Add practical deployment features: smaller weights through INT8 quantization and a minimal server path that can be measured under load.

Why this is one clean pass:

Quantization and serving are both applied-system extensions on top of the CPU/KV core. They should not be started before the minimum strong version exists.

Expected output:

- INT8 weight format
- Quantization tool
- INT8 runtime loading path
- INT8 correctness or output-drift tests
- Minimal HTTP generation server
- Server load test
- Latency/concurrency result files

Validation:

- INT8 round-trip tests pass.
- Runtime can load INT8 fixtures.
- INT8 benchmark reports memory, latency, and output drift honestly.
- HTTP smoke test works.
- Load test results are reproducible and documented.

Commit timing:

Commit INT8 after quantized loading and tests pass. Commit serving after server smoke/load tests pass.

Suggested commit messages:

```bash
git commit -m "Add INT8 quantization and runtime path"
git commit -m "Benchmark INT8 memory and output drift"
git commit -m "Add minimal HTTP server and load test"
```

---

# Wave 8 — Integration Wave

Prompts:

- Prompt 033 — Reproducible Benchmark Harness
- Prompt 034 — External Engine Comparison Protocol
- Prompt 035 — Final Benchmark Report And README Polish
- Prompt 036 — Single-Token Inference Technical Writeup
- Prompt 037 — KV Cache Performance Writeup
- Prompt 038 — CUDA Bottleneck Analysis Writeup
- Prompt 039 — Final Independent Roadmap Verification

Goal:

Integrate all implemented work into a final application-ready project. This wave should make the repository easy to review, reproduce, and discuss in an MS application, interview, or project portfolio.

Why this is the final wave:

The integration wave depends on everything before it. It should not introduce major new features. Its job is to connect the implementation, benchmarks, README, reports, and technical writeups into one coherent artifact.

Expected output:

- Unified benchmark harness
- Final benchmark result directory
- External engine comparison protocol
- Final benchmark report
- README polish
- Single-token inference walkthrough
- KV cache performance analysis
- CUDA bottleneck analysis
- Final independent validation pass

Validation:

- Full test suite passes, or failures are documented with exact cause.
- Benchmark commands are reproducible.
- README, report, and result files agree.
- No unsupported claims remain.
- Deferred features are labeled as future work.
- Progress tracker final status is set to one of:
  - Minimum Strong Version Complete
  - Excellent Version Complete
  - Incomplete With Listed Gaps

Commit timing:

Commit benchmark/report integration only after results and README agree. Commit final validation after the independent verification pass completes.

Suggested commit messages:

```bash
git commit -m "Add reproducible benchmark harness and final report"
git commit -m "Add TinyServe technical writeups"
git commit -m "Complete final TinyServe integration validation"
```

---

# Recommended Implementation Order

| Order | Wave | Prompt Range | Commit Style |
| --- | --- | --- | --- |
| 1 | Wave 0 — Project Foundation And Execution Control | 000-003 | One foundation commit |
| 2 | Wave 1 — Model Boundary, Weights, And Tokenization | 004, 006-008 | One commit or three small commits |
| 3 | Wave 2 — CPU Transformer Math Core | 005, 009-011 | One math-core commit |
| 4 | Wave 3 — CPU Forward Pass And Generation CLI | 012-014 | One vertical-slice commit |
| 5 | Wave 4 — Correctness Harness Against References | 015-017 | One correctness commit |
| 6 | Wave 5 — KV Cache And Minimum Strong Version | 018-022 | Two or three commits if benchmarks take time |
| 7 | Wave 6 — Optional CUDA Acceleration Track | 023-027 | Hardware-gated commits |
| 8 | Wave 7 — Quantization And Serving Layer | 028-032 | Two or three feature commits |
| 9 | Wave 8 — Integration Wave | 033-039 | Final report, writeup, and validation commits |

---

# Best Application Impact Version

For MS applications, the minimum strong version should include Waves 0-5 and Wave 8.

That gives:

- A from-scratch C++/systems inference project
- CPU transformer implementation
- Correctness harness
- KV cache
- Reproducible benchmarks
- Technical writeups

The excellent version includes Waves 6-7 as well:

- CUDA-gated acceleration
- INT8 quantization
- Minimal serving layer
- Load testing

If time becomes tight, prioritize in this order:

1. Wave 0
2. Wave 1
3. Wave 2
4. Wave 3
5. Wave 4
6. Wave 5
7. Wave 8
8. Wave 7
9. Wave 6

Reason:

For a Georgia Tech-focused MS profile, correctness, systems reasoning, benchmarking discipline, and KV cache analysis matter more than unfinished CUDA or serving features. A smaller but deeply verified inference engine is stronger than a broad project with weak evidence.

