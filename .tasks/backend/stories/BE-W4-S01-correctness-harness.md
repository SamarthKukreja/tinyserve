# BE-W4-S01 — Correctness Harness Against References

Source prompts:

- Prompt 015 — Offline Reference Dump Tool
- Prompt 016 — Layer-Level Correctness Tests Against References
- Prompt 017 — Correctness Documentation And Real-Model Gate

Goal:

Add evidence that TinyServe's implementation matches reference outputs at meaningful checkpoints.

Implementation contract:

- Add offline reference dump tooling.
- Add saved fixture references.
- Add layer-level and logit comparison tests where feasible.
- Document tolerances.
- Gate real-model correctness unless all required artifacts are available.

Validation:

- Fixture reference tests pass.
- Tolerance policy is documented.
- Real-model validation status is explicit.

Suggested commit:

```bash
git commit -m "Add reference correctness harness"
```

