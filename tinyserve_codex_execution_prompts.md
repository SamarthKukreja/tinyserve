# TinyServe Codex Execution Prompts

This file contains a sequential set of Codex-ready prompts for building TinyServe from scratch. Execute them in order, starting with Prompt 000. Each prompt is designed as a meaningful development milestone, not a micro-task.

Source documents:

- `tinyserve_ms_roadmap.md`
- `tinyserve_codex_timeline.md`

Project framing:

```text
TinyServe is a minimal C++/CUDA LLM inference runtime built to study transformer decoding, KV caching, quantization, and GPU bottlenecks. It prioritizes correctness, measurement, and reproducibility over production-level speed.
```

Do not implement TinyServe while reading this file. Use each prompt as a standalone Codex task.

---

## Phase 0 — Repository Skeleton And Scope

### Prompt 000 — Initialize Progress Tracker

```text
Initialize the TinyServe implementation progress tracker only. Do not implement TinyServe yet.

Before doing anything:

1. Read `tinyserve_ms_roadmap.md` completely.
2. Read `tinyserve_codex_timeline.md` completely.
3. Confirm whether `tinyserve_implementation_progress.md` already exists.

Objective:

Create `tinyserve_implementation_progress.md` initialized from the full roadmap and timeline.

The tracker must contain:

# TinyServe Implementation Progress

## Overall Progress

- Total prompts
- Completed prompts
- Remaining prompts
- Percentage complete
- Current phase
- Current milestone
- Current prompt

## Phase Progress

For every phase:

- Status
- Completed prompts
- Remaining prompts
- Timeline target
- Notes

## Milestone Progress

For every roadmap milestone:

- Status
- Related prompts
- Completion criteria
- Notes

## Implementation Progress

Create a sequential checklist of every generated prompt from Prompt 001 through the final validation prompt.

## Roadmap Coverage

Track every major roadmap requirement and the prompt or prompts responsible for implementing it.

## Timeline Progress

Track:

- Current phase
- Planned milestone
- Completed milestone
- Next milestone
- Schedule status

## Current Task

- Current Prompt
- Current Phase
- Current Milestone
- Current Objective

## Change Log

Record:

- Date
- Prompt number
- Phase
- Work completed
- Files created/modified
- Tests added
- Notes

Constraints:

- Do not implement project code.
- Do not create the TinyServe repository skeleton yet.
- Do not modify `tinyserve_ms_roadmap.md`.
- Do not modify `tinyserve_codex_timeline.md`.
- Do not mark implementation prompts complete.

Validation:

- Verify the tracker includes all phases and prompts.
- Verify the current task points to Prompt 001.
- Verify all prompt statuses are initially unchecked or pending.

Stop after creating and initializing the tracker.
```

### Prompt 001 — Project Foundation And Repository Skeleton

```text
Implement the first TinyServe milestone: project foundation and repository skeleton.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 0 in `tinyserve_ms_roadmap.md`.
3. Read Phase 0 in `tinyserve_codex_timeline.md`.
4. Confirm Prompt 000 is complete.
5. Inspect the workspace to ensure a TinyServe repo/folder does not already contain conflicting user work.

Objective:

Create the initial TinyServe project structure and a minimal buildable C++ project.

Implementation:

- Create the `tinyserve/` project root if it does not exist.
- Add `README.md` with the honest educational/study-runtime positioning.
- Add `.gitignore`.
- Add `CMakeLists.txt`.
- Create directories:
  - `docs/`
  - `runtime/`
  - `tools/`
  - `tests/`
  - `bench/`
  - `examples/`
- Add a minimal placeholder C++ executable target, such as `runtime/main.cpp`, that prints a TinyServe version/banner.
- Add a simple test target or test placeholder that can be run locally.

Constraints:

- Do not implement transformer inference yet.
- Do not download models.
- Do not add CUDA yet.
- Do not claim production readiness.
- Preserve any existing user files.

Testing:

- Configure/build the project using the intended local build command.
- Run the placeholder executable.
- Run the placeholder test command if present.

Validation:

- Verify the repo structure matches the roadmap.
- Verify the project builds.
- Verify the README does not overclaim.

Progress tracking:

After completing the milestone:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 001 complete.
- Update Phase 0 progress.
- Add a Change Log entry with files created.

Stop after completing this milestone.
```

### Prompt 002 — Scope, Model Choice, And Benchmark Protocol Docs

```text
Implement the next TinyServe milestone: scope, model choice, and benchmark protocol documentation.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 0 in `tinyserve_ms_roadmap.md`.
3. Read Phase 0 in `tinyserve_codex_timeline.md`.
4. Review the current TinyServe project skeleton.
5. Confirm Prompt 001 is complete.

Objective:

Add the Phase 0 documentation that explains what TinyServe is, what it is not, why the model is chosen, and how benchmarks will be measured.

Implementation:

- Create or update:
  - `docs/scope.md`
  - `docs/model_choice.md`
  - `docs/transformer_inference_notes.md`
  - `docs/kv_cache_notes.md`
  - `docs/benchmark_protocol.md`
- Document the allowed runtime dependencies and offline-only tooling.
- Document that PyTorch/Hugging Face may be used only for offline reference generation or weight conversion.
- Document the preferred model path:
  - primary: Qwen2.5-0.5B or Qwen2.5-1.5B
  - secondary: Llama-3.2-1B
- Document benchmark metrics:
  - time to first token
  - prefill latency
  - decode tokens/sec
  - peak memory
  - correctness tolerance
- Add a short "what TinyServe will not do" section.

Constraints:

- Do not implement runtime code in this prompt.
- Do not invent benchmark numbers.
- Do not download model weights.
- Keep the writing technical and honest.

Testing:

- Run any existing documentation lint/check command if available.
- Run the current build/test command to ensure docs did not disturb the project.

Validation:

- Verify the docs cover all Phase 0 roadmap documentation requirements.
- Verify there are no claims such as "faster than vLLM" or "production-ready."

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 002 complete.
- Update roadmap coverage for scope, model choice, inference notes, KV cache notes, and benchmark protocol.
- Add a Change Log entry.

Stop after completing this milestone.
```

### Prompt 003 — Developer Tooling, Build Targets, And Baseline Test Harness

```text
Implement the next TinyServe milestone: baseline developer tooling and test harness.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 0 in both roadmap and timeline files.
3. Review current `CMakeLists.txt`, README, and project structure.
4. Confirm Prompts 001 and 002 are complete.

Objective:

Make the empty TinyServe project easy to build, test, and extend before runtime implementation begins.

Implementation:

- Add build targets for:
  - main executable
  - unit tests
  - optional benchmark executable placeholder
- Add a tiny test framework choice suitable for a C++ project:
  - use a simple in-repo minimal test runner, or
  - use standard CMake/CTest if available
- Add a `tests/` placeholder test that verifies the test runner works.
- Add a `bench/` placeholder benchmark executable or script that records "not implemented yet" without fake numbers.
- Add basic error/status utilities if useful, but keep them minimal.
- Update README with build/test commands.

Constraints:

- Do not add heavyweight dependencies unless necessary.
- Do not implement inference yet.
- Do not add CUDA yet.
- Do not make benchmark claims.

Testing:

- Run configure/build.
- Run the test target.
- Run the placeholder benchmark only if it is explicitly safe and clearly marked as a placeholder.

Validation:

- Verify a future Codex prompt can add runtime components without reworking the build system.
- Verify README commands match actual commands.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 003 complete.
- Update Phase 0 as complete if all Phase 0 criteria are satisfied.
- Add a Change Log entry.

Stop after completing this milestone.
```

---

## Phase 1 — Correct CPU Inference Baseline

### Prompt 004 — Model Config Schema And TinyServe Weight Format

```text
Implement the next TinyServe milestone: model config schema and TinyServe weight format documentation.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 in `tinyserve_ms_roadmap.md`.
3. Read Phase 1 in `tinyserve_codex_timeline.md`.
4. Review the current project implementation.
5. Confirm Phase 0 is complete.

Objective:

Define the model configuration data structure and the custom TinyServe weight-file layout used by the runtime.

Implementation:

- Add:
  - `runtime/model_config.hpp`
  - `runtime/model_config.cpp`
  - `docs/weights.md`
- Define a minimal model config structure with fields needed for Qwen/Llama-style decoder-only inference:
  - vocab size
  - hidden size
  - intermediate size
  - number of layers
  - number of heads
  - number of KV heads if applicable
  - RoPE base or relevant RoPE fields
  - norm epsilon
  - max sequence length
  - dtype/precision metadata
- Define the TinyServe weight-file concept:
  - config/header section
  - tensor metadata
  - row-major tensor body
  - dtype and shape information
- Implement config parsing from a simple JSON-like or line-based format, using only dependencies already accepted by the project.
- Add tests for config parsing and validation.

Constraints:

- Do not implement weight conversion yet beyond the schema needed for tests.
- Do not download real model weights.
- Keep the format simple and documented.

Testing:

- Add unit tests for valid and invalid configs.
- Run the full current test suite.

Validation:

- Verify the config schema can support the selected small decoder-only model family.
- Verify `docs/weights.md` is consistent with the code.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 004 complete.
- Record files and tests added.

Stop after completing this milestone.
```

### Prompt 005 — CPU Tensor Utilities And Numeric Primitives

```text
Implement the next TinyServe milestone: CPU tensor utilities and numeric primitives.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 in both source roadmap files.
3. Review Prompt 004 outputs.
4. Confirm model config tests pass.

Objective:

Add the minimal CPU tensor infrastructure needed for transformer layer implementation.

Implementation:

- Add:
  - `runtime/tensor.hpp`
  - `runtime/tensor.cpp`
- Implement a small tensor abstraction or simple typed buffer wrapper.
- Support:
  - shape metadata
  - contiguous row-major storage
  - basic indexing helpers
  - shape validation
  - allocation and views where useful
- Add helper functions for:
  - vector dot product
  - matrix-vector multiply baseline
  - elementwise add
  - scalar operations
  - simple dtype conversion hooks if needed
- Add tests for shape handling and numeric helper correctness.

Constraints:

- Keep it simple and readable.
- Do not build a full NumPy clone.
- Do not add GPU code.
- Do not implement transformer layers yet.

Testing:

- Add unit tests for tensor shapes, indexing, and numeric operations.
- Run all tests.

Validation:

- Verify the tensor utilities can support RMSNorm, attention, MLP, and logits in later prompts.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 005 complete.
- Add a Change Log entry.

Stop after completing this milestone.
```

### Prompt 006 — Weight Conversion Tool With Dummy Fixture

```text
Implement the next TinyServe milestone: offline weight conversion tool and dummy fixture support.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read the Phase 1 weight-loading requirements in both source roadmap files.
3. Review `docs/weights.md`, model config code, and tensor utilities.
4. Confirm Prompts 004 and 005 are complete.

Objective:

Create an offline tool that can write TinyServe-format model files, starting with a deterministic dummy fixture so tests can run without downloading real model weights.

Implementation:

- Add:
  - `tools/convert_weights.py`
  - `tests/fixtures/` or another small fixture location
- The tool should be able to:
  - write a tiny deterministic model fixture
  - write config/header metadata
  - write named tensors with shapes and dtype metadata
- If real HF/safetensors support is sketched, keep it behind explicit arguments and do not require downloads.
- Update `docs/weights.md` with the conversion command and fixture format.

Constraints:

- Do not download real model weights.
- Do not require PyTorch/HF just to generate the dummy fixture.
- PyTorch/HF support may be optional and offline-only.
- Keep binary fixtures small enough for Git.

Testing:

- Add tests that generate or load the dummy fixture.
- Run all tests.

Validation:

- Verify the fixture can be used by the future C++ weight loader.
- Verify the conversion script does not make unsupported assumptions about real model layout.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 006 complete.
- Record generated fixture files if committed.

Stop after completing this milestone.
```

### Prompt 007 — C++ Weight Loader And Fixture Loading Tests

```text
Implement the next TinyServe milestone: C++ weight loader.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 weight loading requirements.
3. Review `tools/convert_weights.py`, `docs/weights.md`, model config code, and tensor utilities.
4. Confirm Prompt 006 is complete.

Objective:

Implement the runtime-side loader for TinyServe-format weight files.

Implementation:

- Add:
  - `runtime/weight_loader.hpp`
  - `runtime/weight_loader.cpp`
- Load:
  - config/header metadata
  - tensor names
  - tensor shapes
  - tensor dtype metadata
  - tensor buffers
- Provide lookup APIs for named tensors.
- Validate missing tensors, shape mismatches, and malformed files with clear errors.
- Integrate the loader into the build.
- Add tests that load the dummy fixture and inspect config/tensor metadata.

Constraints:

- Do not implement model forward pass yet.
- Do not silently ignore malformed files.
- Do not use PyTorch/HF in runtime.

Testing:

- Run weight loader unit tests.
- Run all existing tests.

Validation:

- Verify fixture load works.
- Verify failure cases produce understandable errors.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 007 complete.
- Update milestone progress for weight loading.

Stop after completing this milestone.
```

### Prompt 008 — Tokenizer Boundary And Temporary Adapter

```text
Implement the next TinyServe milestone: tokenizer boundary and temporary tokenizer adapter.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 tokenizer expectations in the roadmap and timeline.
3. Review current CLI/build structure.
4. Confirm weight loading is complete.

Objective:

Define how TinyServe handles tokenization without letting tokenizer implementation block the core inference milestone.

Implementation:

- Add:
  - `runtime/tokenizer.hpp`
  - `runtime/tokenizer.cpp`
  - `docs/tokenizer.md`
- Define a tokenizer interface used by the runtime.
- Implement a simple temporary tokenizer adapter suitable for tests and dummy fixtures.
- If a full C++ tokenizer is not implemented now, document it as a deliberate staged choice.
- Add tests for encode/decode roundtrip on the temporary tokenizer.
- Ensure the runtime can accept either:
  - raw token IDs for tests, or
  - prompt text through the temporary adapter.

Constraints:

- Do not spend this prompt implementing a full production BPE tokenizer.
- Do not use HF tokenization inside the C++ runtime.
- Do not block model-forward work on full tokenizer support.

Testing:

- Add tokenizer interface tests.
- Run all tests.

Validation:

- Verify later prompts can drive model inference with token IDs even if full tokenizer work is deferred.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 008 complete.
- Record the tokenizer limitation clearly.

Stop after completing this milestone.
```

### Prompt 009 — CPU RMSNorm And RoPE

```text
Implement the next TinyServe milestone: CPU RMSNorm and RoPE.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 CPU layer requirements.
3. Review tensor utilities and model config fields.
4. Confirm tokenizer boundary and weight loader are complete.

Objective:

Implement CPU RMSNorm and RoPE primitives with tests.

Implementation:

- Add or update:
  - `runtime/layers.hpp`
  - `runtime/layers.cpp`
  - `runtime/rope.hpp` if useful
  - `runtime/rope.cpp` if useful
- Implement RMSNorm with configurable epsilon.
- Implement RoPE rotation for query/key vectors using model config parameters.
- Add tests for:
  - RMSNorm output on small deterministic inputs
  - RoPE shape/position behavior
  - edge cases such as zero vectors or short dimensions

Constraints:

- Keep CPU implementation straightforward and readable.
- Do not optimize prematurely.
- Do not add CUDA.

Testing:

- Run layer unit tests.
- Run full test suite.

Validation:

- Verify numeric tolerances are documented in tests or docs.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 009 complete.
- Record tests added.

Stop after completing this milestone.
```

### Prompt 010 — CPU Attention Masking And Softmax

```text
Implement the next TinyServe milestone: CPU attention masking and softmax.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 attention requirements.
3. Review RMSNorm/RoPE code and tensor utilities.
4. Confirm Prompt 009 is complete.

Objective:

Implement the baseline CPU attention primitives needed for single-sequence decoder-only inference.

Implementation:

- Add or update:
  - `runtime/attention.hpp`
  - `runtime/attention.cpp`
- Implement:
  - causal masking
  - numerically stable softmax
  - scaled dot-product attention for small CPU baseline
  - support for multi-head shape handling
  - support for grouped-query/MQA metadata if the selected config requires it, even if simplified initially
- Add tests for:
  - softmax stability
  - causal masking
  - simple attention outputs on deterministic inputs
  - shape validation

Constraints:

- Correctness over speed.
- No KV cache yet.
- No CUDA yet.

Testing:

- Run attention tests.
- Run all tests.

Validation:

- Verify attention outputs are deterministic and shape-safe.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 010 complete.

Stop after completing this milestone.
```

### Prompt 011 — CPU MLP, SwiGLU, Residuals, And Layer Composition

```text
Implement the next TinyServe milestone: CPU MLP/SwiGLU, residuals, and transformer layer composition.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 CPU runtime requirements.
3. Review implemented RMSNorm, RoPE, and attention primitives.
4. Confirm Prompt 010 is complete.

Objective:

Implement the remaining CPU transformer-layer pieces and compose a single decoder block.

Implementation:

- Update:
  - `runtime/layers.hpp`
  - `runtime/layers.cpp`
  - `runtime/attention.hpp`
  - `runtime/attention.cpp`
- Implement:
  - MLP projection path
  - SwiGLU or the activation required by the selected model
  - residual adds
  - per-layer forward function
  - clear shape and tensor-name expectations
- Add tests for:
  - activation output on small inputs
  - MLP shape behavior
  - residual composition
  - one-layer dummy forward pass

Constraints:

- Use dummy fixture tensors for tests.
- Do not implement full model loop yet unless needed for a one-layer test.
- Do not add KV cache yet.

Testing:

- Run layer composition tests.
- Run all tests.

Validation:

- Verify the single-block implementation can be reused by the full model forward prompt.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 011 complete.

Stop after completing this milestone.
```

### Prompt 012 — Full CPU Model Forward Pass

```text
Implement the next TinyServe milestone: full CPU model forward pass.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 CPU inference deliverables.
3. Review model config, weight loader, tokenizer boundary, and transformer layer code.
4. Confirm Prompts 004 through 011 are complete.

Objective:

Integrate the CPU components into a full decoder-only model forward pass for a single sequence.

Implementation:

- Add:
  - `runtime/model.hpp`
  - `runtime/model.cpp`
- Implement:
  - embedding lookup
  - per-layer loop
  - final normalization
  - LM head/logits
  - forward pass for a sequence of token IDs
- Use the dummy fixture to validate end-to-end execution.
- Add tests for:
  - full model construction from loaded weights
  - forward pass shape
  - deterministic logits on dummy fixture

Constraints:

- No KV cache in this prompt.
- No CUDA.
- No benchmark claims yet.

Testing:

- Run full model tests.
- Run all tests.

Validation:

- Verify a sequence of token IDs produces logits.
- Verify missing tensors fail clearly.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 012 complete.

Stop after completing this milestone.
```

### Prompt 013 — Sampling And Greedy Decoding

```text
Implement the next TinyServe milestone: sampling and greedy decoding.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 decode-loop requirements.
3. Review full CPU model forward implementation.
4. Confirm Prompt 012 is complete.

Objective:

Implement token selection and an autoregressive decode loop for the CPU baseline.

Implementation:

- Add or update:
  - `runtime/sampling.hpp`
  - `runtime/sampling.cpp`
  - `runtime/generator.hpp`
  - `runtime/generator.cpp`
- Implement:
  - greedy argmax decoding
  - temperature sampling
  - top-k or top-p sampling if manageable
  - max token limit
  - optional deterministic seed for sampling
- Add tests for:
  - argmax selection
  - temperature behavior
  - deterministic seeded sampling
  - decode loop termination by `max_tokens`

Constraints:

- Keep sampling simple and correct.
- Do not add server or streaming.
- Do not add KV cache yet.

Testing:

- Run sampling/generator tests.
- Run all tests.

Validation:

- Verify fixed dummy model inputs produce stable greedy outputs.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 013 complete.

Stop after completing this milestone.
```

### Prompt 014 — Generation CLI End-To-End CPU Baseline

```text
Implement the next TinyServe milestone: generation CLI for the CPU baseline.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 1 deliverable command in the roadmap.
3. Review model forward and generator code.
4. Confirm Prompts 004 through 013 are complete.

Objective:

Expose the CPU baseline through a command-line interface.

Implementation:

- Add or update:
  - `runtime/generate.cpp`
  - README usage section
  - examples if useful
- Support a command similar to:

  ```bash
  tinyserve generate --model ./models/qwen-0.5b.tserve --prompt "Explain attention" --max-tokens 64
  ```

- If only dummy fixtures are available, support:

  ```bash
  tinyserve generate --model tests/fixtures/tiny_dummy.tserve --prompt "hello" --max-tokens 8
  ```

- Add CLI argument validation.
- Add an integration test or scripted smoke test for the CLI.

Constraints:

- Do not download real model weights.
- Do not claim real model quality if running only fixture mode.
- Do not add KV cache yet.

Testing:

- Build the CLI.
- Run the CLI smoke test.
- Run all tests.

Validation:

- Verify the command works from README instructions.
- Verify README clearly states the current baseline and limitations.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 014 complete.
- Mark Phase 1 CPU baseline complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 2 — Reference Correctness Harness

### Prompt 015 — Offline Reference Dump Tool

```text
Implement the next TinyServe milestone: offline Hugging Face reference dump tool.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 2 in the roadmap and timeline.
3. Review current CPU baseline and weight conversion format.
4. Confirm Phase 1 is complete.

Objective:

Create an offline tool that can generate reference activations/logits from Hugging Face for fixed prompts, without making Hugging Face part of the TinyServe runtime path.

Implementation:

- Add:
  - `tools/dump_reference.py`
  - `docs/correctness.md` initial section
- The tool should:
  - accept a local HF model path or model identifier
  - accept fixed prompts or a prompt file
  - dump selected reference outputs to a documented fixture directory
  - record model name, dtype, prompt, layer selection, and generation settings
- Add safety text:
  - stop/ask before downloading large models
  - reference dumps may be generated locally rather than committed if large
- Provide a tiny mock/reference mode for CI that does not require downloading a real model.

Constraints:

- Do not download models automatically.
- Do not use reference outputs as the runtime implementation.
- Do not require GPUs for basic tests.

Testing:

- Test the mock/reference mode.
- Run existing tests.

Validation:

- Verify `docs/correctness.md` explains how to generate real references later.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 015 complete.

Stop after completing this milestone.
```

### Prompt 016 — Layer-Level Correctness Tests Against References

```text
Implement the next TinyServe milestone: layer-level correctness tests.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 2 correctness requirements.
3. Review `tools/dump_reference.py` and CPU layer implementations.
4. Confirm Prompt 015 is complete.

Objective:

Add tests that compare TinyServe primitives and selected intermediate outputs against deterministic expected/reference data.

Implementation:

- Add or update:
  - `tests/test_rmsnorm.py` or equivalent C++ test
  - `tests/test_rope.py`
  - `tests/test_attention_mask.py`
  - `tests/test_logits.py`
  - `docs/correctness.md`
- Use small deterministic fixtures.
- Compare:
  - RMSNorm outputs
  - RoPE outputs
  - causal mask behavior
  - attention output on a small known input
  - full model logits on dummy fixture
- Document tolerances and why they are acceptable.

Constraints:

- Keep tests runnable without real model downloads.
- Do not loosen tolerances merely to pass broken code.
- Do not add benchmark work in this prompt.

Testing:

- Run all correctness tests.
- Run full test suite.

Validation:

- Verify correctness failures are clear and actionable.
- Verify docs reflect the exact test command.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 016 complete.

Stop after completing this milestone.
```

### Prompt 017 — Correctness Documentation And Real-Model Gate

```text
Implement the next TinyServe milestone: correctness documentation and real-model validation gate.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 2 in both roadmap and timeline.
3. Review all correctness tests and reference tooling.
4. Confirm Prompts 015 and 016 are complete.

Objective:

Finish the correctness harness milestone by documenting how TinyServe should be validated against a real small model when the user approves model access.

Implementation:

- Expand `docs/correctness.md` with:
  - mock fixture correctness path
  - real model reference path
  - exact commands
  - expected output files
  - numerical tolerance policy
  - what to do when outputs diverge
- Update README with a short correctness section.
- Add a TODO/gate note that real model reference generation requires human approval if it downloads large files.
- Add a lightweight validation script if useful, such as `tools/check_reference_outputs.py`, that validates expected files exist and shape metadata matches.

Constraints:

- Do not download a real model.
- Do not mark real-model correctness complete unless actual references exist.
- Do not change runtime functionality unless fixing issues found by tests.

Testing:

- Run all tests.
- Run any new validation script in mock/fixture mode.

Validation:

- Verify the correctness story is clear to an external reviewer.
- Verify progress tracker marks Phase 2 complete only for implemented fixture/reference harness, not unrun real downloads.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 017 complete.
- Mark Phase 2 complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 3 — KV Cache And Minimum Strong Version

### Prompt 018 — Baseline No-Cache Decode Benchmark

```text
Implement the next TinyServe milestone: no-cache decode benchmark baseline.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 3 in the roadmap and timeline.
3. Review the CPU generation CLI and correctness harness.
4. Confirm Phase 2 is complete.

Objective:

Create a reproducible benchmark baseline for the CPU no-cache decode path before adding KV cache.

Implementation:

- Add or update:
  - `bench/kv_cache_bench.cpp` or an initial benchmark harness
  - `bench/results.csv`
  - `docs/kv_cache_results.md` initial baseline section
- Measure or record:
  - prompt length
  - generated token count
  - no-cache decode time
  - no-cache decode tokens/sec
  - optional memory measurement if available
- Use script-generated numbers only.
- If using dummy fixture only, clearly label results as fixture/baseline and not real-model performance.

Constraints:

- Do not implement KV cache yet.
- Do not invent numbers.
- Do not compare against external engines yet.

Testing:

- Run benchmark command.
- Run all tests.

Validation:

- Verify `bench/results.csv` and docs agree.
- Verify benchmark protocol is repeatable.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 018 complete.

Stop after completing this milestone.
```

### Prompt 019 — Contiguous KV Cache Data Structure

```text
Implement the next TinyServe milestone: contiguous KV cache data structure.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 3 KV cache requirements.
3. Review attention code and no-cache decode benchmark.
4. Confirm Prompt 018 is complete.

Objective:

Implement a simple contiguous KV cache for autoregressive decoding.

Implementation:

- Add:
  - `runtime/kv_cache.hpp`
  - `runtime/kv_cache.cpp`
- Support:
  - cache allocation by layer, head, sequence length, head dimension
  - append key/value for each decode step
  - retrieve cached keys/values for attention
  - reset/free cache
  - shape validation
- Add tests for:
  - allocation
  - append/retrieve
  - boundary checks
  - reset behavior

Constraints:

- Implement contiguous cache only.
- Do not implement paged KV cache yet.
- Do not integrate into decode loop in this prompt unless required for tests.

Testing:

- Run KV cache unit tests.
- Run all tests.

Validation:

- Verify data layout is documented in comments or docs.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 019 complete.

Stop after completing this milestone.
```

### Prompt 020 — Integrate KV Cache Into Decode Loop

```text
Implement the next TinyServe milestone: integrate KV cache into autoregressive decoding.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 3 integration requirements.
3. Review KV cache data structure and attention implementation.
4. Confirm Prompt 019 is complete.

Objective:

Make the generation path support cached decoding while preserving no-cache correctness.

Implementation:

- Update:
  - `runtime/attention.cpp`
  - `runtime/model.cpp`
  - `runtime/generator.cpp`
  - CLI options if useful
- Add a decode mode flag:
  - no-cache
  - KV-cache
- Ensure cached greedy generation matches no-cache greedy generation for fixed prompts/fixtures.
- Add tests for:
  - cached output equals no-cache output
  - cache grows by one step during decode
  - repeated runs reset cache correctly

Constraints:

- Do not implement paged cache.
- Do not optimize prematurely.
- Preserve no-cache path.

Testing:

- Run KV cache integration tests.
- Run full test suite.
- Run CLI smoke test in both no-cache and KV-cache mode.

Validation:

- Verify outputs match for deterministic settings.
- Verify README or docs mention the cache mode.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 020 complete.

Stop after completing this milestone.
```

### Prompt 021 — KV Cache Benchmark And Results Documentation

```text
Implement the next TinyServe milestone: KV cache benchmark and results documentation.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 3 benchmark requirements.
3. Review no-cache benchmark and KV-cache integration.
4. Confirm Prompt 020 is complete.

Objective:

Measure and document the latency/memory tradeoff of KV cache vs no-cache decoding.

Implementation:

- Update:
  - `bench/kv_cache_bench.cpp`
  - `bench/results.csv`
  - `docs/kv_cache_results.md`
  - README summary if appropriate
- Benchmark:
  - no-cache mode
  - KV-cache mode
  - same prompt/token settings
  - median of repeated runs if practical
- Record:
  - TTFT or prefill time if available
  - decode tokens/sec
  - peak memory if available
  - model/fixture used
  - hardware context if known
- Explain why KV cache improves decode speed and what memory cost it adds.

Constraints:

- Do not exaggerate results.
- Do not compare against external engines yet.
- Do not manually edit numbers after benchmark generation.

Testing:

- Run all tests.
- Run KV benchmark.

Validation:

- Verify docs and `bench/results.csv` agree.
- Verify limitations are explicit if using dummy/fixture benchmarks.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 021 complete.
- Mark Phase 3 complete if acceptance criteria are met.

Stop after completing this milestone.
```

### Prompt 022 — Minimum Strong Version Report And README Pass

```text
Implement the next TinyServe milestone: minimum strong version report and README pass.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read the "Minimum Strong Version" sections in both roadmap and timeline.
3. Review current CPU runtime, correctness harness, KV cache, and benchmark docs.
4. Confirm Phase 3 is complete.

Objective:

Package the CPU + correctness + KV-cache implementation as a coherent minimum strong MS-application artifact before advanced CUDA/quantization work begins.

Implementation:

- Update:
  - `README.md`
  - `docs/final_benchmark_report.md` initial/minimum version section
  - `docs/kv_cache_results.md` if needed
  - `bench/results.csv` only through benchmark scripts
- Add:
  - architecture overview
  - build/test commands
  - generate command
  - correctness explanation
  - KV cache benchmark table
  - limitations
  - next steps
- Add a suggested tag note such as `v0.3-kv-cache`, but do not create a Git tag unless the user asks.

Constraints:

- Do not claim CUDA/INT8/server support yet.
- Do not claim real-model results if only fixture results exist.
- Do not implement new runtime features in this prompt unless fixing documentation-discovered bugs.

Testing:

- Run full test suite.
- Run minimum benchmark command.

Validation:

- Verify README, docs, and benchmark CSV agree.
- Verify the project would already be explainable on a resume as the minimum strong version.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 022 complete.
- Update timeline status for "minimum strong version."

Stop after completing this milestone.
```

---

## Phase 4 — CUDA Microkernels

### Prompt 023 — CUDA Build Gate And Hardware Detection

```text
Implement the next TinyServe milestone: CUDA build gate and hardware detection.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 4 in both roadmap and timeline.
3. Review current CMake/build setup.
4. Confirm the minimum strong version is complete.

Objective:

Prepare the project for optional CUDA microkernels without breaking CPU-only builds.

Implementation:

- Update `CMakeLists.txt` to support optional CUDA builds.
- Add hardware/environment detection documentation:
  - `docs/cuda_notes.md`
- Add a small CUDA availability check target if CUDA is present.
- Ensure CPU-only build remains supported.
- Document how to set GPU architecture flags.

Human review gate:

- Stop and ask before installing CUDA dependencies.
- Stop and ask if local CUDA availability is unclear.

Constraints:

- Do not implement kernels yet.
- Do not make CUDA required for CPU functionality.

Testing:

- Run CPU-only build/test.
- If CUDA is available, run CUDA configure smoke test.

Validation:

- Verify CPU-only users can still build.
- Verify CUDA setup instructions are honest.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 023 complete.

Stop after completing this milestone.
```

### Prompt 024 — CUDA RMSNorm Kernel And Tests

```text
Implement the next TinyServe milestone: CUDA RMSNorm kernel.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 4 CUDA requirements.
3. Review CPU RMSNorm implementation and CUDA build gate.
4. Confirm Prompt 023 is complete.

Objective:

Implement and test a CUDA RMSNorm microkernel as the first GPU systems milestone.

Implementation:

- Add:
  - `cuda/rmsnorm.cu`
  - `runtime/cuda_ops.hpp`
  - `runtime/cuda_ops.cpp`
  - tests or benchmark support for CUDA RMSNorm
- Compare CUDA RMSNorm output against CPU RMSNorm.
- Add tolerance documentation.
- Keep the CPU path intact.

Constraints:

- Do not require CUDA tests to run on systems without CUDA.
- Do not optimize beyond a clear correct baseline unless easy.
- Do not add RoPE/GEMV in this prompt.

Testing:

- Run CPU tests.
- Run CUDA RMSNorm test if CUDA is available.

Validation:

- Verify CUDA output is numerically close to CPU output.
- Verify fallback behavior if CUDA is not available.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 024 complete.

Stop after completing this milestone.
```

### Prompt 025 — CUDA RoPE Kernel And Tests

```text
Implement the next TinyServe milestone: CUDA RoPE kernel.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 4 CUDA requirements.
3. Review CPU RoPE and CUDA RMSNorm implementation.
4. Confirm Prompt 024 is complete.

Objective:

Implement and test a CUDA RoPE microkernel.

Implementation:

- Add:
  - `cuda/rope.cu`
  - CUDA wrapper functions as needed
  - CUDA RoPE tests or microbench support
- Compare CUDA RoPE output against CPU RoPE on deterministic inputs.
- Document assumptions about shape, head dimension, position, and dtype.

Constraints:

- Keep CUDA optional.
- Do not integrate into full inference path unless this is simple and safe.
- Do not add GEMV yet.

Testing:

- Run CPU tests.
- Run CUDA RoPE tests if CUDA is available.

Validation:

- Verify numerical closeness to CPU.
- Verify docs mention any limitations.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 025 complete.

Stop after completing this milestone.
```

### Prompt 026 — CUDA GEMV Microbenchmark And cuBLAS Baseline

```text
Implement the next TinyServe milestone: CUDA GEMV microbenchmark and cuBLAS comparison.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 4 CUDA benchmark requirements.
3. Review CUDA build support and existing microkernels.
4. Confirm Prompt 025 is complete.

Objective:

Add a decode-oriented GEMV microbenchmark and compare a naive CUDA implementation with a cuBLAS baseline if cuBLAS is available.

Implementation:

- Add:
  - `cuda/gemv.cu`
  - `bench/cuda_microbench.cpp`
  - `docs/cuda_notes.md` benchmark section
- Benchmark:
  - CPU baseline if available
  - naive CUDA GEMV
  - cuBLAS GEMV if available and approved
- Record:
  - input sizes
  - dtype
  - runtime
  - estimated bandwidth if practical

Human review gate:

- Ask before adding or requiring cuBLAS if the toolchain does not already provide it.

Constraints:

- Do not claim production-level GEMM performance.
- Do not hand-edit benchmark numbers.
- Keep benchmarks reproducible.

Testing:

- Run all CPU tests.
- Run CUDA microbench if CUDA is available.

Validation:

- Verify benchmark output is recorded in a machine-readable or documented format.
- Verify docs explain why decode GEMV can be memory-bound.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 026 complete.

Stop after completing this milestone.
```

### Prompt 027 — CUDA Bottleneck Documentation And Optional Runtime Hooks

```text
Implement the next TinyServe milestone: CUDA bottleneck documentation and optional runtime hooks.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 4 acceptance criteria.
3. Review CUDA RMSNorm, RoPE, and GEMV microbench outputs.
4. Confirm Prompt 026 is complete.

Objective:

Complete Phase 4 by documenting CUDA results and optionally wiring safe CUDA hooks into the runtime where appropriate.

Implementation:

- Update:
  - `docs/cuda_notes.md`
  - README advanced/CUDA section
  - `bench/results.csv` or CUDA benchmark output file if used
- If safe, add runtime flags to call CUDA RMSNorm/RoPE for microbench or inference experiments.
- Document:
  - what sped up
  - what did not
  - kernel launch overhead
  - memory bandwidth observations
  - why TinyServe is not expected to beat production engines

Constraints:

- Do not make CUDA path mandatory.
- Do not claim superiority without evidence.
- Do not break CPU inference.

Testing:

- Run CPU tests.
- Run CUDA tests/benchmarks if available.

Validation:

- Verify Phase 4 roadmap requirements are covered.
- Verify docs are backed by actual benchmark outputs.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 027 complete.
- Mark Phase 4 complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 5 — INT8 Quantization

### Prompt 028 — Weight-Only INT8 Quantization Format And Tool

```text
Implement the next TinyServe milestone: INT8 quantization format and offline tool.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 5 in the roadmap and timeline.
3. Review the weight format and conversion tool.
4. Confirm Phase 3 is complete, and Phase 4 if CUDA work was attempted.

Objective:

Add a simple weight-only INT8 quantization tool and extend the weight format to store scales.

Implementation:

- Add:
  - `tools/quantize_weights.py`
  - update `docs/weights.md`
  - update `docs/quantization_results.md` initial section
- Implement per-channel symmetric INT8 quantization for eligible weight tensors.
- Store:
  - INT8 weights
  - scale values
  - metadata indicating quantized tensors
- Add tests using dummy fixture weights.

Constraints:

- Do not implement INT4.
- Do not quantize activations.
- Do not hide quantization error.
- Do not require real model downloads.

Testing:

- Run quantizer tests on small fixtures.
- Run all tests.

Validation:

- Verify quantized files can be inspected and metadata is correct.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 028 complete.

Stop after completing this milestone.
```

### Prompt 029 — INT8 Runtime Path

```text
Implement the next TinyServe milestone: INT8 runtime path.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 5 runtime requirements.
3. Review quantized weight format and CPU matmul/GEMV paths.
4. Confirm Prompt 028 is complete.

Objective:

Make TinyServe load and use weight-only INT8 tensors through dequantization during inference.

Implementation:

- Add:
  - `quantization/int8.hpp`
  - `quantization/int8.cpp`
- Update weight loader and matmul/GEMV path to support:
  - FP16/float baseline weights
  - INT8 weights + scales
  - dequantize-in-register or simple dequantize-before-use path
- Add tests for:
  - dequantization correctness
  - INT8 fixture forward pass
  - output drift vs FP baseline on tiny fixture

Constraints:

- Keep INT8 path simple and correct.
- Do not claim quality preservation.
- Preserve FP path.

Testing:

- Run quantization runtime tests.
- Run all tests.

Validation:

- Verify FP and INT8 paths can both run.
- Verify output drift is measurable.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 029 complete.

Stop after completing this milestone.
```

### Prompt 030 — INT8 Benchmark And Quality Drift Report

```text
Implement the next TinyServe milestone: INT8 benchmark and quality drift report.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 5 benchmark/reporting requirements.
3. Review INT8 runtime path.
4. Confirm Prompt 029 is complete.

Objective:

Measure and document the memory, latency, and output-drift tradeoffs of INT8 quantization.

Implementation:

- Add or update:
  - `bench/quantization_bench.cpp`
  - `docs/quantization_results.md`
  - `bench/results.csv` or a quantization-specific results file
  - README quantization section
- Benchmark:
  - FP baseline
  - INT8 path
- Record:
  - memory/file-size reduction
  - decode tokens/sec
  - output/logit drift on fixture or reference prompts
  - limitations

Constraints:

- Do not manually invent numbers.
- Do not claim INT8 is always faster.
- Do not attempt INT4 in this prompt.

Testing:

- Run full test suite.
- Run quantization benchmark.

Validation:

- Verify benchmark results and docs agree.
- Verify degradation is described honestly.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 030 complete.
- Mark Phase 5 complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 6 — Minimal Serving Layer

### Prompt 031 — Minimal HTTP Generation Server

```text
Implement the next TinyServe milestone: minimal HTTP generation server.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 6 in the roadmap and timeline.
3. Review the generation CLI and runtime APIs.
4. Confirm CPU generation is complete.

Objective:

Expose TinyServe generation through a minimal local HTTP endpoint.

Implementation:

- Add:
  - `server/http_server.cpp`
  - `server/request_queue.cpp` or a minimal queue abstraction
  - build target for the server
- Implement endpoint:

  ```bash
  POST /generate
  ```

- Accept JSON-like input containing:
  - prompt
  - max_tokens
  - optional decoding options
- Return generated text and basic metadata.
- Update README with server usage.

Constraints:

- Keep serving minimal.
- Do not implement production auth, scaling, distributed serving, or SSE yet.
- Do not break CLI generation.

Testing:

- Add a simple server smoke test or local script if feasible.
- Run all unit tests.
- Build server target.

Validation:

- Verify local request returns generated text.
- Verify README says this is minimal serving, not production serving.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 031 complete.

Stop after completing this milestone.
```

### Prompt 032 — Server Load Test And Latency Reporting

```text
Implement the next TinyServe milestone: server load test and latency reporting.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 6 measurement requirements.
3. Review the HTTP server implementation.
4. Confirm Prompt 031 is complete.

Objective:

Add a reproducible load test for the minimal server and document latency/throughput results.

Implementation:

- Add:
  - `bench/server_load_test.py`
  - server benchmark results output
  - README or docs server benchmark section
- Measure:
  - single-request latency
  - concurrent request behavior
  - p50 latency
  - p95 latency
  - aggregate tokens/sec if feasible
- Keep inputs deterministic and small.

Constraints:

- Do not overclaim production serving performance.
- Do not hand-edit benchmark numbers.
- Do not add complex continuous batching unless already stable and explicitly requested.

Testing:

- Run all tests.
- Run server smoke test.
- Run load test if environment supports local server execution.

Validation:

- Verify docs explain hardware/model/fixture context.
- Verify limitations are clear.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 032 complete.
- Mark Phase 6 complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 7 — Final Benchmark And Report

### Prompt 033 — Reproducible Benchmark Harness

```text
Implement the next TinyServe milestone: reproducible benchmark harness.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 7 in the roadmap and timeline.
3. Review existing KV, CUDA, quantization, and server benchmarks.
4. Confirm at least the minimum strong version is complete.

Objective:

Create a unified benchmark harness that can reproduce TinyServe's reported latency/memory results.

Implementation:

- Add:
  - `bench/reproduce.sh` or Windows-compatible equivalent plus notes
  - `bench/results.csv`
  - benchmark README section
- Include commands for:
  - CPU no-cache
  - CPU KV-cache
  - CUDA microbench if available
  - INT8 if available
  - server load test if available
- Record hardware and environment metadata where possible.
- Use median-of-3 or clearly documented repeated-run protocol.

Constraints:

- Do not add external engine comparisons yet unless already available and approved.
- Do not invent results.
- Do not silently overwrite historical results without noting the run.

Testing:

- Run the benchmark harness in the locally supported mode.
- Run all tests.

Validation:

- Verify results are machine-readable.
- Verify README does not contain stale numbers.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 033 complete.

Stop after completing this milestone.
```

### Prompt 034 — External Engine Comparison Protocol

```text
Implement the next TinyServe milestone: external engine comparison protocol.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 7 comparison requirements.
3. Review `bench/reproduce.sh` and current benchmark results.
4. Confirm Prompt 033 is complete.

Objective:

Add a fair comparison protocol for Hugging Face Transformers, llama.cpp, and vLLM where locally feasible, without forcing unsupported installations.

Implementation:

- Add documentation and scripts under `bench/` for:
  - Hugging Face Transformers baseline
  - llama.cpp baseline
  - vLLM baseline only if hardware/tooling supports it
- Each comparison must specify:
  - model
  - precision
  - prompt set
  - batch size
  - max tokens
  - hardware
  - number of runs
- If an engine is not feasible locally, document why and leave it as "not run" rather than fabricating results.

Human review gate:

- Ask before installing external engines or downloading additional model formats.

Constraints:

- Do not use external engines as TinyServe backend.
- Do not claim benchmark superiority unless measured.
- Do not make comparisons asymmetric without explaining the asymmetry.

Testing:

- Run all TinyServe tests.
- Run only comparison scripts that are available locally and approved.

Validation:

- Verify comparison protocol is fair and reproducible.
- Verify any missing comparison is documented honestly.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 034 complete.

Stop after completing this milestone.
```

### Prompt 035 — Final Benchmark Report And README Polish

```text
Implement the next TinyServe milestone: final benchmark report and README polish.

Before coding:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 7 reporting requirements.
3. Review all benchmark outputs and docs.
4. Confirm Prompts 033 and 034 are complete, or document why external comparisons are not run.

Objective:

Turn the implementation and measurements into an admissions-ready repository artifact.

Implementation:

- Create or finalize:
  - `docs/final_benchmark_report.md`
  - `README.md`
  - `bench/results.csv`
- README should include:
  - project positioning
  - architecture diagram or text architecture
  - build/test commands
  - generate command
  - correctness summary
  - KV cache results
  - CUDA/INT8/server status if implemented
  - benchmark table
  - limitations
  - future work
- Report should include:
  - benchmark protocol
  - hardware
  - results
  - bottleneck analysis
  - honest comparison against external engines where available

Constraints:

- Do not claim unimplemented features.
- Do not claim production readiness.
- Do not edit benchmark numbers manually to look better.

Testing:

- Run full test suite.
- Run benchmark harness or document why a full benchmark cannot be rerun.

Validation:

- Verify README numbers match benchmark outputs.
- Verify final report and README agree.
- Verify every limitation is clear.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 035 complete.
- Mark Phase 7 complete if acceptance criteria are met.

Stop after completing this milestone.
```

---

## Phase 8 — Technical Writing

### Prompt 036 — Single-Token Inference Technical Writeup

```text
Implement the next TinyServe milestone: single-token inference technical writeup.

Before writing:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 8 in the roadmap and timeline.
3. Review implemented CPU inference code and docs.
4. Confirm CPU baseline is implemented.

Objective:

Create a human technical note explaining what happens when TinyServe generates one token.

Implementation:

- Add:
  - `docs/blog_single_token_inference.md`
- Explain:
  - tokenization boundary
  - embedding
  - RMSNorm
  - RoPE
  - attention
  - MLP
  - logits
  - sampling/greedy selection
- Include:
  - what was expected
  - what broke or was tricky
  - what was measured
  - what could be improved

Constraints:

- Do not write marketing copy.
- Do not claim features not implemented.
- Keep the voice technical and personal enough to show real understanding.

Testing:

- Run tests to ensure no code was broken if docs references changed.

Validation:

- Verify the writeup matches current code behavior.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 036 complete.

Stop after completing this milestone.
```

### Prompt 037 — KV Cache Performance Writeup

```text
Implement the next TinyServe milestone: KV cache performance writeup.

Before writing:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 8 and Phase 3 documentation requirements.
3. Review KV cache implementation and benchmark results.
4. Confirm KV cache benchmark is complete.

Objective:

Create a technical note explaining KV cache behavior and measured tradeoffs in TinyServe.

Implementation:

- Add:
  - `docs/blog_kv_cache.md`
- Explain:
  - no-cache decoding
  - cached decoding
  - why recomputing K/V is expensive
  - memory cost of cache
  - benchmark setup
  - measured speed difference
  - limitations of contiguous cache
- Include:
  - what was expected
  - what broke
  - what was measured
  - future paged-cache idea

Constraints:

- Use actual benchmark outputs.
- Do not claim paged attention is implemented unless it is.

Testing:

- Run full tests if any links/scripts are changed.

Validation:

- Verify the writeup matches `docs/kv_cache_results.md` and `bench/results.csv`.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 037 complete.

Stop after completing this milestone.
```

### Prompt 038 — CUDA Bottleneck Analysis Writeup

```text
Implement the next TinyServe milestone: CUDA bottleneck analysis writeup.

Before writing:

1. Read `tinyserve_implementation_progress.md`.
2. Read Phase 8 and Phase 4 documentation requirements.
3. Review CUDA microkernels and benchmark outputs.
4. Confirm CUDA work is complete or explicitly marked as deferred.

Objective:

Create a technical note explaining TinyServe CUDA bottlenecks and what the microbenchmarks showed.

Implementation:

- Add:
  - `docs/blog_cuda_bottlenecks.md`
- If CUDA was implemented, explain:
  - RMSNorm kernel
  - RoPE kernel
  - GEMV/cuBLAS comparison
  - memory bandwidth observations
  - kernel launch overhead
  - why production engines are faster
- If CUDA was deferred, explain:
  - what blocked it
  - what the planned implementation would measure
  - why CPU/KV-cache correctness remained the priority

Constraints:

- Do not invent profiler results.
- Do not claim CUDA speedups without benchmark evidence.
- Keep the analysis honest.

Testing:

- Run relevant tests if docs or build metadata changed.

Validation:

- Verify the writeup matches actual CUDA status.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 038 complete.
- Mark Phase 8 complete if all required writeups are complete.

Stop after completing this milestone.
```

---

## Final Validation

### Prompt 039 — Final Independent Roadmap Verification

```text
Perform the final TinyServe validation milestone. Do not assume the project is complete merely because earlier prompts are marked complete.

Before changing anything:

1. Read `tinyserve_ms_roadmap.md` completely.
2. Read `tinyserve_codex_timeline.md` completely.
3. Read `tinyserve_implementation_progress.md` completely.
4. Review the entire TinyServe implementation.
5. Review README, docs, tests, benchmarks, and generated results.

Objective:

Independently verify TinyServe against the complete roadmap and timeline, fix legitimate gaps, and produce a final completion status.

Implementation:

- Check project structure against the roadmap.
- Check every phase:
  - Phase 0 scope/docs/build skeleton
  - Phase 1 CPU inference baseline
  - Phase 2 correctness harness
  - Phase 3 KV cache and benchmarks
  - Phase 4 CUDA microkernels or documented deferral
  - Phase 5 INT8 quantization or documented status
  - Phase 6 minimal server or documented status
  - Phase 7 final benchmark report
  - Phase 8 technical writeups
- Verify every benchmark claim is backed by a script or results file.
- Verify every README claim matches implementation status.
- Verify no forbidden claims exist:
  - faster than vLLM
  - production-ready
  - state-of-the-art
  - full inference engine
- Run the complete test suite.
- Run required benchmarks that are locally feasible.
- If gaps are found, fix only legitimate implementation/documentation gaps needed for roadmap consistency.

Constraints:

- Do not invent benchmark numbers.
- Do not download models or install external engines without user approval.
- Do not hide incomplete stretch goals; mark them as future work.
- Do not rewrite unrelated code.

Validation:

- Full test suite passes, or any failures are documented with cause.
- Benchmark outputs and README agree.
- Progress tracker accurately reflects completed and deferred work.
- Final report clearly states minimum strong version or excellent version status.

Progress tracking:

- Update `tinyserve_implementation_progress.md`.
- Mark Prompt 039 complete.
- Update all phase and milestone statuses.
- Add a final Change Log entry.
- Set final completion status to one of:
  - Minimum Strong Version Complete
  - Excellent Version Complete
  - Incomplete With Listed Gaps

Stop after final validation and report the status.
```

---

# Commit Timing And Message Plan

Use this table as the default commit checklist while executing the prompts.

Rules:

- Commit only after the prompt's validation passes or the prompt explicitly documents an honest deferral.
- Do not commit broken tests, invented benchmark numbers, or half-updated benchmark documents.
- If a prompt only discovers a blocker and produces no file changes, update the progress tracker only if useful, then skip the commit.
- If a prompt naturally becomes large, split it into the smaller commits already suggested inside that prompt; otherwise use the message below.
- Every commit should leave `tinyserve_implementation_progress.md` updated with the prompt status, validation result, and any blocker.

| Prompt | Commit When | Commit Message |
| --- | --- | --- |
| 000 | After the progress tracker file exists and has the full prompt/phase checklist. | `Initialize TinyServe implementation progress tracker` |
| 001 | After the repository skeleton, README, license, `.gitignore`, and placeholder tests/build commands are coherent. | `Add TinyServe project foundation` |
| 002 | After scope, model choice, inference notes, benchmark protocol, and constraints are documented. | `Add TinyServe scope and benchmark planning docs` |
| 003 | After the local build/test tooling runs successfully on the initial skeleton. | `Add baseline build and test tooling` |
| 004 | After config parsing, schema validation, weight format documentation, and tests pass. | `Add model config schema and weight format docs` |
| 005 | After CPU tensor utilities compile and shape/indexing tests pass. | `Add CPU tensor utilities` |
| 006 | After the offline converter can create a deterministic fixture and fixture tests pass. | `Add offline weight conversion fixture` |
| 007 | After the C++ weight loader reads the fixture and validates tensor metadata. | `Add TinyServe weight loader` |
| 008 | After the tokenizer boundary and fixture tokenizer adapter are tested. | `Add tokenizer interface and fixture adapter` |
| 009 | After RMSNorm and RoPE implementations match hand-computed or fixture references. | `Implement CPU RMSNorm and RoPE` |
| 010 | After causal attention, masking, and softmax tests pass. | `Implement CPU attention masking and softmax` |
| 011 | After MLP/SwiGLU and decoder block composition tests pass. | `Implement CPU MLP and decoder block composition` |
| 012 | After the full CPU model forward pass works on the fixture and regression tests pass. | `Add full CPU model forward pass` |
| 013 | After greedy/top-k/top-p sampling tests pass and decoding is deterministic under fixed seeds. | `Add sampling and greedy decode loop` |
| 014 | After the CLI smoke test generates text from the fixture model and CPU baseline tests pass. | `Add CPU generation CLI` |
| 015 | After the offline reference dump tool is documented and works on fixtures or is honestly gated for real models. | `Add offline reference dump tool` |
| 016 | After layer-level correctness tests compare TinyServe outputs to saved references. | `Add layer-level correctness tests` |
| 017 | After the real-model correctness gate is documented with exact commands, tolerance policy, and current status. | `Document real-model correctness validation gate` |
| 018 | After the no-cache decode benchmark runs and results are saved with machine/environment details. | `Add no-cache decode benchmark baseline` |
| 019 | After the contiguous KV cache data structure has unit tests for indexing, capacity, and reset behavior. | `Add contiguous KV cache data structure` |
| 020 | After cached decoding matches no-cache decoding on fixture prompts. | `Integrate KV cache into decode loop` |
| 021 | After KV cache benchmarks are saved and README/report numbers match the result files. | `Benchmark KV cache latency and memory tradeoffs` |
| 022 | After the minimum strong version is packaged with passing tests, honest README status, and reproducible commands. | `Package TinyServe minimum strong version` |
| 023 | After optional CUDA build detection works and CPU-only builds remain unaffected. | `Add optional CUDA build gate` |
| 024 | After the CUDA RMSNorm kernel passes correctness checks or is documented as deferred because CUDA is unavailable. | `Add CUDA RMSNorm kernel` |
| 025 | After the CUDA RoPE kernel passes correctness checks or is documented as deferred because CUDA is unavailable. | `Add CUDA RoPE kernel` |
| 026 | After GEMV/cuBLAS microbenchmark results are recorded, or the CUDA benchmark is honestly skipped with reason. | `Add CUDA GEMV microbenchmark` |
| 027 | After CUDA runtime hooks and bottleneck notes match actual implementation status. | `Document CUDA bottlenecks and runtime hooks` |
| 028 | After INT8 quantization format, quantizer, and round-trip tests pass. | `Add INT8 quantization format and tool` |
| 029 | After INT8 runtime loading/inference tests pass on fixtures. | `Add INT8 runtime path` |
| 030 | After INT8 memory, latency, and output-drift results are saved and documented. | `Benchmark INT8 memory and output drift` |
| 031 | After the minimal HTTP server smoke test works with the CLI/runtime path. | `Add minimal HTTP generation server` |
| 032 | After the server load test records latency and concurrency behavior. | `Add server load test and latency report` |
| 033 | After the unified benchmark harness runs all implemented benchmark modes reproducibly. | `Add reproducible benchmark harness` |
| 034 | After the external engine comparison protocol is documented without unapproved installs or unsupported claims. | `Add external engine comparison protocol` |
| 035 | After the final benchmark report, README, and generated result files agree. | `Add final benchmark report and README polish` |
| 036 | After the single-token inference walkthrough reflects the implemented code path. | `Write single-token inference walkthrough` |
| 037 | After the KV cache analysis reflects the actual benchmark results and tradeoffs. | `Write KV cache performance analysis` |
| 038 | After the CUDA bottleneck analysis reflects the real CUDA status, including deferrals if applicable. | `Write CUDA bottleneck analysis` |
| 039 | After final verification passes or all remaining gaps are explicitly documented. | `Complete final TinyServe roadmap validation` |

---

# Roadmap → Prompt Coverage Matrix

| Roadmap Requirement | Prompt(s) | Timeline Phase | Status |
| --- | --- | --- | --- |
| Progress tracker before implementation | 000 | Pre-Phase / Phase 0 | Covered |
| Project skeleton and repository structure | 001 | Phase 0 | Covered |
| README with honest positioning | 001, 022, 035 | Phase 0, Phase 3, Phase 7 | Covered |
| Scope documentation | 002 | Phase 0 | Covered |
| Model choice documentation | 002 | Phase 0 | Covered |
| Transformer inference notes | 002, 036 | Phase 0, Phase 8 | Covered |
| KV cache notes | 002, 021, 037 | Phase 0, Phase 3, Phase 8 | Covered |
| Benchmark protocol | 002, 018, 033, 035 | Phase 0, Phase 3, Phase 7 | Covered |
| Build/test developer tooling | 003 | Phase 0 | Covered |
| Model config schema | 004 | Phase 1 | Covered |
| TinyServe weight format | 004, 006, 007 | Phase 1 | Covered |
| Offline conversion tool | 006 | Phase 1 | Covered |
| C++ weight loader | 007 | Phase 1 | Covered |
| Tokenizer boundary/adapter | 008 | Phase 1 | Covered |
| CPU tensor utilities | 005 | Phase 1 | Covered |
| CPU RMSNorm | 009 | Phase 1 | Covered |
| CPU RoPE | 009 | Phase 1 | Covered |
| CPU attention and causal masking | 010 | Phase 1 | Covered |
| CPU MLP/SwiGLU/residuals | 011 | Phase 1 | Covered |
| Full CPU model forward pass | 012 | Phase 1 | Covered |
| Greedy decoding and sampling | 013 | Phase 1 | Covered |
| Generation CLI | 014 | Phase 1 | Covered |
| Offline HF reference dump tool | 015 | Phase 2 | Covered |
| Layer/logit correctness tests | 016 | Phase 2 | Covered |
| Correctness documentation and real-model gate | 017 | Phase 2 | Covered |
| Baseline no-cache benchmark | 018 | Phase 3 | Covered |
| Contiguous KV cache | 019 | Phase 3 | Covered |
| KV cache integration | 020 | Phase 3 | Covered |
| KV cache benchmark and results | 021 | Phase 3 | Covered |
| Minimum strong version packaging | 022 | Phase 3 / Phase 7 partial | Covered |
| Optional CUDA build support | 023 | Phase 4 | Covered |
| CUDA RMSNorm kernel | 024 | Phase 4 | Covered |
| CUDA RoPE kernel | 025 | Phase 4 | Covered |
| CUDA GEMV/cuBLAS microbenchmark | 026 | Phase 4 | Covered |
| CUDA bottleneck documentation | 027, 038 | Phase 4, Phase 8 | Covered |
| INT8 quantization format/tool | 028 | Phase 5 | Covered |
| INT8 runtime path | 029 | Phase 5 | Covered |
| INT8 benchmark and quality drift | 030 | Phase 5 | Covered |
| Minimal HTTP server | 031 | Phase 6 | Covered |
| Server load test and latency reporting | 032 | Phase 6 | Covered |
| Unified benchmark harness | 033 | Phase 7 | Covered |
| External engine comparison protocol | 034 | Phase 7 | Covered |
| Final benchmark report | 035 | Phase 7 | Covered |
| Single-token inference writeup | 036 | Phase 8 | Covered |
| KV cache writeup | 037 | Phase 8 | Covered |
| CUDA bottleneck writeup | 038 | Phase 8 | Covered |
| Final independent validation | 039 | Final Validation | Covered |
| Stretch goals documented but not blocking | 022, 035, 039 | Phase 7 / Final | Covered |

# Timeline → Prompt Coverage Matrix

| Timeline Phase | Milestone | Prompt(s) | Status |
| --- | --- | --- | --- |
| Prompt 000 | Persistent progress tracker | 000 | Covered |
| Phase 0 | Repository skeleton and scope | 001-003 | Covered |
| Phase 1 | Correct CPU inference baseline | 004-014 | Covered |
| Phase 2 | Reference correctness harness | 015-017 | Covered |
| Phase 3 | KV cache and benchmarks | 018-021 | Covered |
| Phase 3 / Phase 7 partial | Minimum strong version packaging | 022 | Covered |
| Phase 4 | CUDA build support and microkernels | 023-027 | Covered |
| Phase 5 | INT8 quantization | 028-030 | Covered |
| Phase 6 | Minimal HTTP serving | 031-032 | Covered |
| Phase 7 | Benchmark harness and final report | 033-035 | Covered |
| Phase 8 | Technical writeups | 036-038 | Covered |
| Final Validation | Independent roadmap verification | 039 | Covered |
