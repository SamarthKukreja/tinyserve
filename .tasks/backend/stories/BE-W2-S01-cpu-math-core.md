# BE-W2-S01 — CPU Transformer Math Core

Source prompts:

- Prompt 005 — CPU Tensor Utilities And Numeric Primitives
- Prompt 009 — CPU RMSNorm And RoPE
- Prompt 010 — CPU Attention Masking And Softmax
- Prompt 011 — CPU MLP, SwiGLU, Residuals, And Layer Composition

Goal:

Implement the CPU math core for a decoder-only transformer in a clear, testable way.

Implementation contract:

- Add tensor utilities and shape/index helpers.
- Implement RMSNorm and RoPE.
- Implement causal attention, masking, and softmax.
- Implement MLP/SwiGLU and decoder block composition.
- Prioritize correctness over speed.

Validation:

- Unit tests pass for tensor utilities.
- RMSNorm/RoPE tests match fixture or hand-computed references.
- Attention mask behavior is tested.
- Decoder block tests are deterministic.

Suggested commit:

```bash
git commit -m "Implement CPU transformer math core"
```

