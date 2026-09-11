# BE-W7-S01 — Quantization And Serving Layer

Source prompts:

- Prompt 028 — Weight-Only INT8 Quantization Format And Tool
- Prompt 029 — INT8 Runtime Path
- Prompt 030 — INT8 Benchmark And Quality Drift Report
- Prompt 031 — Minimal HTTP Generation Server
- Prompt 032 — Server Load Test And Latency Reporting

API contract:

- `docs/api-spec.md` is canonical.
- `.tasks/API-SPEC.md` is a working pointer only.

Goal:

Add practical deployment-oriented extensions: INT8 quantization and a minimal measured serving path.

Implementation contract:

- Add INT8 weight format and quantization tool.
- Add INT8 runtime path.
- Benchmark memory, latency, and output drift.
- Add minimal HTTP server implementing the canonical API only.
- Add server smoke/load tests.

Validation:

- INT8 round-trip tests pass.
- INT8 runtime fixture test passes.
- HTTP `/health` and `/generate` smoke tests pass.
- Load-test results are saved and documented.

Suggested commits:

```bash
git commit -m "Add INT8 quantization and runtime path"
git commit -m "Benchmark INT8 memory and output drift"
git commit -m "Add minimal HTTP server and load test"
```

