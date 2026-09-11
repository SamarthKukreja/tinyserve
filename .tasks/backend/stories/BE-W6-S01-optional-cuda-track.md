# BE-W6-S01 — Optional CUDA Acceleration Track

Source prompts:

- Prompt 023 — CUDA Build Gate And Hardware Detection
- Prompt 024 — CUDA RMSNorm Kernel And Tests
- Prompt 025 — CUDA RoPE Kernel And Tests
- Prompt 026 — CUDA GEMV Microbenchmark And cuBLAS Baseline
- Prompt 027 — CUDA Bottleneck Documentation And Optional Runtime Hooks

Goal:

Add a GPU-aware extension without making CUDA mandatory for project success.

Implementation contract:

- Add optional CUDA build detection.
- Keep CPU-only build working.
- Implement CUDA kernels only if toolchain and hardware permit validation.
- Document CUDA deferrals honestly if hardware is unavailable.
- Do not claim end-to-end CUDA acceleration unless wired and benchmarked.

Validation:

- CPU-only build still passes.
- CUDA tests pass when CUDA is available.
- CUDA-unavailable path skips cleanly.
- Bottleneck notes match implementation status.

Suggested commits:

```bash
git commit -m "Add optional CUDA build gate"
git commit -m "Add CUDA RMSNorm and RoPE kernels"
git commit -m "Document CUDA bottlenecks and runtime hooks"
```

CUDA-unavailable fallback commit:

```bash
git commit -m "Document CUDA-gated acceleration path"
```

