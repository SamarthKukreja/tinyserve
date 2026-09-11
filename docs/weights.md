# TinyServe Weight Format

TinyServe uses a deliberately small, versioned binary container for converted model configuration and named row-major tensors. The current implementation validates deterministic synthetic fixtures only. It has not converted or validated a real Qwen or Llama checkpoint.

## Integer and tensor encoding

- All integers and floating-point bodies are little-endian.
- `u32` and `u64` are unsigned 32-bit and 64-bit integers.
- Tensor names and configuration text are UTF-8 bytes without terminators.
- Tensor bodies are dense C-order (row-major) arrays with no padding.
- Dtype code `1` is IEEE-754 `float32`; code `2` reserves IEEE-754 `float16` storage. The Wave 1 fixture uses only `float32`.

## Version 1 layout

The fixed file header is:

| Field | Encoding | Meaning |
| --- | --- | --- |
| Magic | 8 bytes | ASCII `TSERVE01` |
| Format version | `u32` | `1` |
| Config length | `u32` | Number of following configuration bytes |
| Tensor count | `u32` | Number of tensor records |
| Configuration | byte array | Strict `key=value` schema described below |

Each tensor record immediately follows the previous record:

| Field | Encoding | Meaning |
| --- | --- | --- |
| Name length | `u32` | Tensor-name byte count |
| Dtype | `u32` | Dtype code |
| Rank | `u32` | Number of dimensions |
| Body length | `u64` | Tensor-body byte count |
| Name | byte array | UTF-8 tensor name |
| Shape | `rank` × `u64` | Row-major dimensions |
| Body | byte array | Dense tensor data |

The loader rejects bad magic or versions, missing/unknown/duplicate configuration fields, duplicate tensor names, zero dimensions, unsupported dtypes, unsafe metadata sizes, body/shape mismatches, truncation, and trailing bytes. Callers can additionally require an exact name, shape, and dtype through `WeightFile::require_tensor`.

## Model configuration schema

Configuration is newline-delimited `key=value` text. Blank lines and lines beginning with `#` are ignored; keys are otherwise strict.

| Key | Meaning | Validation |
| --- | --- | --- |
| `vocab_size` | Vocabulary entries | Positive `u32` |
| `hidden_size` | Decoder hidden width | Positive and divisible by `num_heads` |
| `intermediate_size` | Gated-MLP intermediate width | Positive `u32` |
| `num_layers` | Decoder-layer count | Positive `u32` |
| `num_heads` | Query-attention heads | Positive `u32` |
| `num_kv_heads` | Key/value heads for MHA/GQA | Positive, no larger than and dividing `num_heads` |
| `rope_theta` | RoPE base | Positive finite number |
| `norm_epsilon` | Normalization epsilon | Positive finite number |
| `max_sequence_length` | Supported context capacity | Positive `u32` |
| `dtype` | Default model precision metadata | `float32` or `float16` |

These fields cover the shared boundary needed for the selected Qwen/Llama-style decoder families. Architecture-specific fields must be added through a new documented format version or a backward-compatible schema change with tests; they must not be guessed by the loader.

## Deterministic fixture conversion

The standard-library-only converter writes a tiny synthetic fixture:

```bash
python tools/convert_weights.py --dummy-output build/fixtures/dummy.tserve
```

The normal test build generates the same fixture under its build directory. CTest writes two independent copies and verifies byte-for-byte determinism; the C++ loader and model tests validate a complete one-layer, twelve-tensor model with a 256-entry byte vocabulary. The fixture remains synthetic and is not evidence of named-model compatibility.

The script does not currently accept Hugging Face or safetensors inputs. Adding that offline conversion path requires an explicit mapping for each supported architecture plus real-model reference validation. PyTorch or Hugging Face libraries will never be runtime dependencies.
