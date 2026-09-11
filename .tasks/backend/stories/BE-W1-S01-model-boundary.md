# BE-W1-S01 — Model Boundary, Weights, And Tokenization

Source prompts:

- Prompt 004 — Model Config Schema And TinyServe Weight Format
- Prompt 006 — Weight Conversion Tool With Dummy Fixture
- Prompt 007 — C++ Weight Loader And Fixture Loading Tests
- Prompt 008 — Tokenizer Boundary And Temporary Adapter

Goal:

Define how external model assets enter TinyServe through config files, converted weights, loader code, and a tokenizer boundary.

Implementation contract:

- Add model config schema and validation.
- Document TinyServe's weight format.
- Add deterministic dummy fixture conversion.
- Add C++ weight loader tests.
- Add tokenizer interface/adapter tests.
- Keep real-model support gated unless actually validated.

Validation:

- Converter fixture is deterministic.
- Loader reads fixture metadata and tensors.
- Tokenizer boundary tests pass.
- Tracker marks Wave 1 complete only after validation.

Suggested commit:

```bash
git commit -m "Add model format, weight loading, and tokenizer boundary"
```

