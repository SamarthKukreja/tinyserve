# CPU Math Core

Wave 2 implements a correctness-first float32 math layer for one decoder block. It is intentionally scalar and dependency-free; it is not an optimized inference path and has not been checked against a real model.

## Tensor and projection conventions

- `Tensor` owns contiguous row-major float32 storage and positive shape dimensions.
- Linear projection weights use `[output_features, input_features]`; optional biases use `[output_features]`, allowing both bias-free Llama-style and biased Qwen query/key/value projections.
- Decoder hidden states use `[sequence, hidden_size]`.
- Projected queries use `[sequence, num_heads, head_dim]`.
- Projected keys and values use `[sequence, num_kv_heads, head_dim]`.
- Grouped-query attention maps each contiguous group of query heads to one KV head; `num_heads` must be divisible by `num_kv_heads`.

## Operator conventions

- RMSNorm computes `x / sqrt(mean(x²) + epsilon)` along the final dimension and then applies the learned weight. A zero vector with zero epsilon is defined to remain zero for the isolated primitive test; validated model configs require a positive epsilon.
- RoPE follows the selected Qwen/Llama-style split-half layout: coordinates in the first half rotate with the corresponding coordinates in the second half. Pair index `i` uses inverse frequency `1 / theta^(2i / head_dim)` and the absolute token position.
- Attention is causal, single-sequence, and no-cache. Scores are scaled by `1 / sqrt(head_dim)` and normalized with max-subtracted softmax.
- The MLP uses `down(silu(gate(x)) * up(x))`.
- The decoder block is pre-norm with residual addition after attention and after the MLP.

## Decoder block weight names

The `DecoderBlockWeights` fields map to these converted tensor names for layer `L`:

- `model.layers.L.input_layernorm.weight`
- `model.layers.L.self_attn.q_proj.weight` and optional `.bias`
- `model.layers.L.self_attn.k_proj.weight` and optional `.bias`
- `model.layers.L.self_attn.v_proj.weight` and optional `.bias`
- `model.layers.L.self_attn.o_proj.weight`
- `model.layers.L.post_attention_layernorm.weight`
- `model.layers.L.mlp.gate_proj.weight`
- `model.layers.L.mlp.up_proj.weight`
- `model.layers.L.mlp.down_proj.weight`

Wave 2 consumes an already assembled `DecoderBlockWeights` object. Wave 3's `Model::from_weights` now performs the strict mapping from named `WeightFile` records into those layer objects.

## Validation limits

Hand-computed unit references use an absolute tolerance of `1e-5`. Tests cover row-major indexing, primitive arithmetic, RMSNorm, RoPE positions, stable softmax, causal masking, grouped-query shape mapping, SwiGLU, residuals, invalid shapes, and a deterministic one-block fixture.

This is fixture-level validation only. Later waves must add a full model loop and offline reference-activation comparisons before TinyServe can claim real-model numerical correctness.
