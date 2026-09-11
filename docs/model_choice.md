# Model Choice

## Initial target: Qwen2.5-0.5B

TinyServe will target Qwen2.5-0.5B first, subject to the Wave 1 model-boundary work and a separate approval before downloading weights. Its modest size makes correctness iterations and CPU experiments more practical than starting with a multi-billion-parameter model, while its architecture still exercises the transformer components the project intends to study.

Qwen2.5-1.5B is a follow-on size after the smaller path is validated. Llama-3.2-1B is a secondary architecture for testing whether the runtime boundaries generalize; it is not part of the initial implementation claim.

## Selection criteria

- Feasible conversion, storage, and test iteration on development hardware
- Architecture details that can be documented and implemented explicitly
- Availability of reproducible offline reference outputs
- A tokenizer boundary that can be isolated from the core runtime
- Licensing and access terms reviewed before artifacts are downloaded or distributed

## Validation gates

Model support is not established by naming a target. TinyServe may claim support only after configuration, converted weights, tokenizer behavior, intermediate activations or logits, and end-to-end generation have passed their planned checks. Fixture-only validation and real-model validation will be labeled separately.

No external model weights have been included or downloaded. The checked-in converter creates only deterministic synthetic test weights.
