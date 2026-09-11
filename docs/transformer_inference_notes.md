# Transformer Inference Notes

Autoregressive inference has two operational stages. Prefill processes the prompt and establishes per-layer state. Decode then produces one token at a time, with each new token depending on all preceding positions.

The planned correctness-first CPU path is:

1. Convert model configuration and weights offline into a documented TinyServe format.
2. Tokenize through an explicit boundary rather than hiding tokenization inside the runtime.
3. Look up embeddings and run decoder layers in order.
4. Apply normalization, rotary position encoding, causal self-attention, the gated MLP, and residual connections according to the selected architecture.
5. Project the final hidden state to logits.
6. Select the next token using greedy decoding first, then documented sampling semantics.
7. Repeat until the requested limit or a supported stop condition.

Tensor shapes, data types, numerical tolerances, and architecture-specific conventions must be made explicit as those components are added. The CPU baseline should remain simple enough to compare against offline reference activations before optimization.

PyTorch and Hugging Face may generate fixtures and reference values offline. They cannot execute any step on behalf of the TinyServe runtime. The Wave 3 executable performs every stage above against synthetic float32 weights and the temporary byte tokenizer, recomputing the full sequence at each decode step. Named-model correctness remains unverified.
