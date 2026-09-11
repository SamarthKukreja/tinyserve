# BE-W0-S01 — Project Foundation And Execution Control

Source prompts:

- Prompt 000 — Initialize Progress Tracker
- Prompt 001 — Project Foundation And Repository Skeleton
- Prompt 002 — Scope, Model Choice, And Benchmark Protocol Docs
- Prompt 003 — Developer Tooling, Build Targets, And Baseline Test Harness

Goal:

Create the TinyServe repository foundation: skeleton, README, scope docs, benchmark protocol, baseline tooling, and a persistent progress tracker.

Implementation contract:

- Create only the minimal project shell needed for future waves.
- Add build/test commands, even if initially lightweight.
- Add `tinyserve_implementation_progress.md`.
- Do not implement transformer kernels or product endpoints in this wave.

Validation:

- Skeleton exists.
- README is honest and does not overclaim.
- Build/test commands are documented.
- `.tasks/backend/TRACKER.md` marks Wave 0 complete only after validation.
- `.tasks/backend/validate.sh` passes.

Suggested commit:

```bash
git commit -m "Add TinyServe project foundation and execution plan"
```

