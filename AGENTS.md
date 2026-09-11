# TinyServe Agent Instructions

This repository is planned as an MS-application-quality ML systems project: a small, honest inference engine with CPU transformer inference, KV cache, correctness checks, benchmarks, and optional CUDA/INT8/server extensions.

Primary planning files:

- `tinyserve_implementation_waves.md`
- `tinyserve_codex_execution_prompts.md`
- `tinyserve_codex_timeline.md`
- `tinyserve_ms_roadmap.md`
- `.tasks/backend/IMPLEMENTATION-ORDER.md`
- `.tasks/backend/TRACKER.md`

Rules for implementation agents:

- Implement exactly the requested wave.
- Read `.tasks/backend/IMPLEMENTATION-ORDER.md` and the selected story file before editing code.
- Do not start later-wave work early.
- Do not invent benchmark numbers or claim real-model support before validation exists.
- Prefer small, readable C++ implementations before optimizing.
- Keep CPU-only builds working even if CUDA support is added later.
- Update `.tasks/backend/TRACKER.md` after each implemented wave.
- Run `.tasks/backend/validate.sh` before handoff.
- If a dependency, GPU, model file, or benchmark run is unavailable, document the blocker honestly.

Commit behavior:

- Commit at wave boundaries after validation passes.
- If a wave includes a long benchmark, commit benchmark code first, then commit measured results after the run completes.
- Use the commit messages recommended in the selected story file unless there is a clear reason to split them.

