# BE-W3-S01 — CPU Forward Pass And Generation CLI

Source prompts:

- Prompt 012 — Full CPU Model Forward Pass
- Prompt 013 — Sampling And Greedy Decoding
- Prompt 014 — Generation CLI End-To-End CPU Baseline

Goal:

Create the first end-to-end CPU generation path using the fixture model.

Implementation contract:

- Add full model forward pass.
- Add greedy decoding.
- Add sampling only if already covered by tests.
- Add CLI generation command.
- Do not claim real-model quality unless real-model validation exists.

Validation:

- Fixture forward pass test passes.
- Greedy decoding is deterministic.
- CLI smoke test generates output.
- Tracker marks Wave 3 complete only after validation.

Suggested commit:

```bash
git commit -m "Add end-to-end CPU generation baseline"
```

