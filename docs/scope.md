# TinyServe Scope

TinyServe is a small study runtime for understanding the mechanics and systems tradeoffs of autoregressive transformer inference. The project favors readable implementations, correctness checks, and reproducible measurements over feature breadth or production performance.

## Planned runtime scope

- Single-sequence CPU transformer inference in C++
- Explicit model configuration and converted weight loading
- Greedy decoding and small, documented sampling primitives
- Reference-based numerical correctness checks
- A contiguous KV cache and before/after measurements
- Optional, gated CUDA microkernels, weight-only INT8, and minimal HTTP serving after the CPU path is correct

The inference path may use the C++ standard library, custom C++/CUDA code, and narrowly justified native dependencies. New runtime dependencies must be documented and must not replace the core implementation being studied.

## Offline-only tooling

Python, PyTorch, Hugging Face Transformers, and safetensors may be used by tools that convert weights, prepare tokenizer artifacts, or generate trusted reference outputs. They must not perform inference behind the TinyServe runtime or CLI. External engines may later be used only as clearly labeled comparison baselines.

## What TinyServe will not do

- Claim production readiness or parity with production engines
- Use `transformers.generate`, PyTorch inference, vLLM, or llama.cpp as its runtime backend
- Begin with batching, distributed inference, paged attention, INT4, or streaming
- Publish performance claims without reproducible commands and saved raw results
- Hide unsupported models, numerical drift, unavailable hardware, or failed experiments

## Current status and constraints

Through Wave 3, TinyServe contains a versioned synthetic model fixture, scalar float32 CPU operators, a no-cache full forward pass, deterministic decoding, and a fixture CLI. The temporary byte tokenizer and synthetic weights do not provide named-model or language-quality support. No reference-validated real checkpoint, KV cache, CUDA kernel, quantized path, benchmark result, or server exists yet.

The target machine's GPU, VRAM, driver, CUDA version, compute capability, CPU, operating-system build, and compiler versions have not yet been recorded. They must be captured from the machine used for any future benchmark before results are reported. The CPU-only CMake build remains the required baseline on every supported environment.
