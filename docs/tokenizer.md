# Tokenizer Boundary

TinyServe exposes tokenization through the C++ `Tokenizer` interface:

- `encode(text)` maps prompt bytes/text to token IDs.
- `decode(token_ids)` maps generated IDs back to text.
- `TokenInput` and `resolve_token_input` let later runtime code accept either prompt text through a tokenizer or caller-supplied raw token IDs.

Wave 1 provides `ByteTokenizer`, a deterministic temporary adapter. It assigns each input byte its unsigned value from 0 through 255 and reverses that mapping during decode. This makes ASCII and UTF-8 byte sequences round-trip exactly and keeps fixtures independent of external packages. It is not BPE, tiktoken, SentencePiece, or a Qwen/Llama-compatible production tokenizer.

Raw token IDs deliberately bypass the temporary adapter. This allows model-forward and layer tests to use known token sequences before a model-specific tokenizer is implemented.

Wave 3 uses this same boundary in the CLI: `--prompt` passes text through `ByteTokenizer`, while `--tokens` supplies comma-separated IDs directly. Only the synthetic 256-entry fixture is compatible with the byte adapter.

## Staged real-model support

A future model tokenizer must implement the same interface and be checked against offline reference token IDs for the exact model artifacts and tokenizer configuration. Hugging Face may generate those references offline, but HF tokenization is not called from the C++ runtime. Until those checks exist, TinyServe claims only byte-tokenizer fixture support—not text-generation compatibility with a named model.
