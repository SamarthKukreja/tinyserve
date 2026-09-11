# TinyServe Backend Implementation Order

This file adapts `tinyserve_implementation_waves.md` into the format expected by the `implement-backend-wave` skill.

Each wave should be implemented independently. The story file listed for a wave is the local implementation contract for that wave.

---

## Wave 0 — Project Foundation And Execution Control

Stories:

- `BE-W0-S01` — `.tasks/backend/stories/BE-W0-S01-project-foundation.md`

Depends on:

- None

Commit message:

```bash
git commit -m "Add TinyServe project foundation and execution plan"
```

---

## Wave 1 — Model Boundary, Weights, And Tokenization

Stories:

- `BE-W1-S01` — `.tasks/backend/stories/BE-W1-S01-model-boundary.md`

Depends on:

- Wave 0 complete

Commit message:

```bash
git commit -m "Add model format, weight loading, and tokenizer boundary"
```

---

## Wave 2 — CPU Transformer Math Core

Stories:

- `BE-W2-S01` — `.tasks/backend/stories/BE-W2-S01-cpu-math-core.md`

Depends on:

- Wave 1 complete

Commit message:

```bash
git commit -m "Implement CPU transformer math core"
```

---

## Wave 3 — CPU Forward Pass And Generation CLI

Stories:

- `BE-W3-S01` — `.tasks/backend/stories/BE-W3-S01-cpu-generation-cli.md`

Depends on:

- Wave 2 complete

Commit message:

```bash
git commit -m "Add end-to-end CPU generation baseline"
```

---

## Wave 4 — Correctness Harness Against References

Stories:

- `BE-W4-S01` — `.tasks/backend/stories/BE-W4-S01-correctness-harness.md`

Depends on:

- Wave 3 complete

Commit message:

```bash
git commit -m "Add reference correctness harness"
```

---

## Wave 5 — KV Cache And Minimum Strong Version

Stories:

- `BE-W5-S01` — `.tasks/backend/stories/BE-W5-S01-kv-cache-minimum-version.md`

Depends on:

- Wave 4 complete

Preferred commit messages:

```bash
git commit -m "Add KV cache decode path"
git commit -m "Benchmark KV cache latency and memory tradeoffs"
git commit -m "Package TinyServe minimum strong version"
```

---

## Wave 6 — Optional CUDA Acceleration Track

Stories:

- `BE-W6-S01` — `.tasks/backend/stories/BE-W6-S01-optional-cuda-track.md`

Depends on:

- Wave 5 complete

Preferred commit messages:

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

## Wave 7 — Quantization And Serving Layer

Stories:

- `BE-W7-S01` — `.tasks/backend/stories/BE-W7-S01-quantization-serving.md`

Depends on:

- Wave 5 complete
- Wave 6 is optional, but if attempted before Wave 7 it must be complete or honestly deferred

Preferred commit messages:

```bash
git commit -m "Add INT8 quantization and runtime path"
git commit -m "Benchmark INT8 memory and output drift"
git commit -m "Add minimal HTTP server and load test"
```

---

## Wave 8 — Integration Wave

Stories:

- `BE-W8-S01` — `.tasks/backend/stories/BE-W8-S01-final-integration.md`

Depends on:

- Wave 5 complete
- Wave 6 and Wave 7 either complete or honestly deferred

Preferred commit messages:

```bash
git commit -m "Add reproducible benchmark harness and final report"
git commit -m "Add TinyServe technical writeups"
git commit -m "Complete final TinyServe integration validation"
```

