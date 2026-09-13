# TinyServe Backend Wave Tracker

Status values:

- `Not Started`
- `In Progress`
- `Complete`
- `Deferred With Reason`
- `Blocked`

| Wave | Story | Status | Validation | Notes |
| --- | --- | --- | --- | --- |
| 0 | `BE-W0-S01` Project foundation | Complete | MSVC Release build; CTest 1/1; planning validation passed | Repository skeleton, honest scope/model/benchmark docs, CMake tooling, and progress tracker |
| 1 | `BE-W1-S01` Model boundary | Complete | MSVC full and runtime-only builds; CTest 5/5; deterministic conversion and malformed-loader cases pass | Strict config schema, versioned fixture format, stdlib converter, validating loader, byte-tokenizer/raw-ID boundary; real-model support gated |
| 2 | `BE-W2-S01` CPU math core | Complete | MSVC full and runtime-only builds; CTest 8/8; hand-computed numeric, masking, shape-failure, GQA, and deterministic block cases pass | Contiguous float32 tensors, RMSNorm, split-half RoPE, stable causal attention, optional projection biases, SwiGLU, residuals, reusable one-block composition |
| 3 | `BE-W3-S01` CPU generation CLI | Complete | MSVC full and runtime-only builds; CTest 12/12; deterministic forward/greedy/seeded sampling; valid and invalid CLI cases pass | Complete one-layer synthetic fixture, strict weight assembly, no-cache forward/decode, prompt/raw-token CLI; real-model correctness and quality unverified |
| 4 | `BE-W4-S01` Correctness harness | Complete | MSVC full and runtime-only builds; CTest 14/14; deterministic reference freshness and C++ primitive/layer/logit comparisons pass | Standard-library fixture oracle, saved references, fixed tolerances, optional local-only HF dump path; real-model validation not run and gated on approval |
| 5 | `BE-W5-S01` KV cache minimum version | Complete | MSVC full and runtime-only builds; CTest 17/17; cached/no-cache logits and greedy outputs match; 18 raw benchmark samples retained | Contiguous single-sequence cache, both CLI modes, generated fixture benchmark/report; 4+8-token fixture measured at 2,746.874351 vs 19,425.612392 decode tok/s with 192 persistent KV bytes; not real-model performance |
| 6 | `BE-W6-S01` Optional CUDA track | Not Started | Pending | CUDA build gate, kernels, microbenchmarks, bottleneck docs |
| 7 | `BE-W7-S01` Quantization and serving | Not Started | Pending | INT8 path, benchmarks, HTTP server, load test |
| 8 | `BE-W8-S01` Final integration | Not Started | Pending | Unified benchmarks, final report, writeups, validation |

## Current Overall Status

`Wave 5 Complete — Wave 6 Ready (Optional CUDA Gate)`

## Wave Closeout Log

Add entries here after each wave:

```text
YYYY-MM-DD — Wave N — Status — Tests/validation — Commit hash or pending commit
```

2026-08-17 — Wave 0 — Complete — MSVC Release build, CTest 1/1, banner and non-measuring benchmark placeholder, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)

2026-08-17 — Wave 1 — Complete — MSVC full and runtime-only builds, CTest 5/5, deterministic converter fixture, config/loader/tokenizer failure cases, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)

2026-08-17 — Wave 2 — Complete — MSVC full and runtime-only builds, CTest 8/8, tensor/RMSNorm/RoPE/attention/SwiGLU/residual/decoder-block cases, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)

2026-08-17 — Wave 3 — Complete — MSVC full and runtime-only builds, CTest 12/12, deterministic fixture forward/generation and valid/invalid CLI cases, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)

2026-08-17 — Wave 4 — Complete — MSVC full and runtime-only builds, CTest 14/14, byte-stable saved reference plus RMSNorm/RoPE/mask/attention/layer/final-hidden/full-logit comparisons, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)

2026-08-17 — Wave 5 — Complete — MSVC full and runtime-only builds, CTest 17/17, cached/no-cache equivalence, 9 repeats per mode with 100 inner iterations and generated CSV/Markdown consistency checks, `.tasks/backend/validate.ps1` passed (`validate.sh` could not launch because WSL returned `E_ACCESSDENIED`) — Pending commit (workspace is not a Git repository)
