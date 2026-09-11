# Correctness harness

Wave 4 adds two deliberately separate correctness paths. The fixture path is reproducible, offline, and runs in CTest. The real-model path is an explicit human-approved experiment and has **not** been run. Passing the fixture harness does not establish Qwen, Llama, tokenizer, or language-quality correctness.

## Offline fixture path

`tools/dump_reference.py fixture` is an independent Python-standard-library oracle. It parses the deterministic `.tserve` fixture and implements scalar RMSNorm, split-half RoPE, causal masking, grouped-query attention, the decoder block, final normalization, and the LM head without calling the C++ runtime.

After a test-enabled build creates `build/fixtures/dummy.tserve`, regenerate and validate the saved artifact with:

```bash
python tools/dump_reference.py fixture \
  --model build/fixtures/dummy.tserve \
  --tokens 65,66 \
  --output tests/fixtures/fixture_reference.txt
python tools/check_reference_outputs.py \
  --reference tests/fixtures/fixture_reference.txt
ctest --test-dir build --output-on-failure -R reference
```

The committed `tests/fixtures/fixture_reference.txt` contains source metadata, token IDs, tolerances, and these checkpoints:

- RMSNorm, split-half RoPE, causal masking, and grouped-query attention primitives
- input embeddings and every fixture decoder-layer output
- final normalized hidden states
- the complete 256-element last-token logit vector

`tinyserve.reference_dump` generates the artifact twice, checks byte-for-byte determinism, validates its schema, and rejects a stale committed reference. `tinyserve.reference_compare` runs the corresponding C++ operations and reports checkpoint/index details when a value diverges.

## Tolerance policy

Fixture comparisons use float32 C++ results against a Python double-precision scalar oracle. For each finite expected value `e` and actual value `a`, the check is:

```text
abs(a - e) <= 2e-5 + 2e-5 * abs(e)
```

Masked negative infinity values must match exactly in sign. NaNs, unexpected infinities, shape changes, missing checkpoints, and stale dumps always fail. These tolerances are intentionally fixed in the reference metadata; loosening them requires an explained reference and documentation change, not an ad hoc test edit.

When a comparison fails:

1. Read the reported checkpoint and first failing flat index.
2. Run `ctest --test-dir build -V -R reference_compare` to inspect maximum errors from earlier checkpoints.
3. Confirm the saved reference is current with `tinyserve.reference_dump`.
4. Check tensor layout, RoPE position/layout, GQA head mapping, mask direction, normalization epsilon, and float accumulation order in that order.
5. Regenerate the committed artifact only when the fixture or intended math contract changed.

## Gated real-model path

The optional Hugging Face mode accepts a local model directory or model ID, one or more prompts, selected layer IDs, dtype, and last-token or full logits. It writes `metadata.json` and one `prompt_NNN.json` file per prompt:

```bash
python tools/dump_reference.py hf \
  --model C:/models/approved-model \
  --prompt "TinyServe reference check" \
  --layers 0,1,3 \
  --dtype float32 \
  --logits last \
  --output-dir build/real-reference
```

This mode imports optional `torch` and `transformers` packages only when invoked. It passes `local_files_only=True` by default, so a missing local model fails instead of downloading. Before using a model ID that is not already local, stop and obtain human approval for the model, license, storage, and network use. Only after that approval may the operator add `--allow-download`; the tool prints a warning when this gate is opened.

Real-model validation status: **not run — waiting for an approved local model/tokenizer and explicit operator authorization**. Therefore Wave 4 completes the fixture correctness harness only; it makes no real-model correctness claim.
