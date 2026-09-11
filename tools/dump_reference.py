#!/usr/bin/env python3
"""Dump reproducible TinyServe fixture or opt-in Hugging Face references.

Fixture mode uses only the Python standard library and is the CI correctness
oracle. Hugging Face mode imports optional dependencies lazily and is local-only
unless the caller explicitly allows a download.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Sequence


MAGIC = b"TSERVE01"
REFERENCE_FORMAT = "tinyserve_reference_v1"


@dataclass(frozen=True)
class Tensor:
    shape: tuple[int, ...]
    values: tuple[float, ...]


def _product(shape: Sequence[int]) -> int:
    result = 1
    for dimension in shape:
        result *= dimension
    return result


def _read_fixture(path: Path) -> tuple[dict[str, str], dict[str, Tensor]]:
    data = memoryview(path.read_bytes())
    offset = 0

    def take(size: int) -> bytes:
        nonlocal offset
        if size < 0 or offset + size > len(data):
            raise ValueError("truncated TinyServe fixture")
        value = bytes(data[offset : offset + size])
        offset += size
        return value

    if take(8) != MAGIC:
        raise ValueError("invalid TinyServe fixture magic")
    version, config_size, tensor_count = struct.unpack("<III", take(12))
    if version != 1:
        raise ValueError(f"unsupported TinyServe fixture version {version}")
    config = {}
    for line in take(config_size).decode("utf-8").splitlines():
        key, value = line.split("=", 1)
        config[key] = value

    tensors: dict[str, Tensor] = {}
    for _ in range(tensor_count):
        name_size, dtype, rank, body_size = struct.unpack("<IIIQ", take(20))
        if dtype != 1:
            raise ValueError("fixture oracle supports float32 tensors only")
        name = take(name_size).decode("utf-8")
        shape = struct.unpack(f"<{rank}Q", take(8 * rank))
        count = _product(shape)
        if body_size != count * 4:
            raise ValueError(f"tensor {name!r} has an invalid byte count")
        values = struct.unpack(f"<{count}f", take(body_size))
        tensors[name] = Tensor(tuple(shape), tuple(values))
    if offset != len(data):
        raise ValueError("TinyServe fixture has trailing bytes")
    return config, tensors


def _linear(input_tensor: Tensor, weights: Tensor) -> Tensor:
    input_width = input_tensor.shape[-1]
    output_width, weight_input = weights.shape
    if input_width != weight_input:
        raise ValueError("linear shape mismatch")
    rows = len(input_tensor.values) // input_width
    output = []
    for row in range(rows):
        for out_column in range(output_width):
            output.append(
                sum(
                    input_tensor.values[row * input_width + column]
                    * weights.values[out_column * input_width + column]
                    for column in range(input_width)
                )
            )
    return Tensor((*input_tensor.shape[:-1], output_width), tuple(output))


def _rms_norm(input_tensor: Tensor, weight: Tensor, epsilon: float) -> Tensor:
    width = input_tensor.shape[-1]
    output = []
    for row in range(len(input_tensor.values) // width):
        values = input_tensor.values[row * width : (row + 1) * width]
        inverse_rms = 1.0 / math.sqrt(sum(value * value for value in values) / width + epsilon)
        output.extend(value * inverse_rms * weight.values[index] for index, value in enumerate(values))
    return Tensor(input_tensor.shape, tuple(output))


def _add(left: Tensor, right: Tensor) -> Tensor:
    return Tensor(left.shape, tuple(a + b for a, b in zip(left.values, right.values)))


def _rope(input_tensor: Tensor, start_position: int, theta: float) -> Tensor:
    sequence, heads, head_dimension = input_tensor.shape
    half = head_dimension // 2
    output = list(input_tensor.values)
    for position in range(sequence):
        for head in range(heads):
            base = (position * heads + head) * head_dimension
            for pair in range(half):
                angle = (start_position + position) / (theta ** ((pair * 2) / head_dimension))
                first = input_tensor.values[base + pair]
                second = input_tensor.values[base + pair + half]
                output[base + pair] = first * math.cos(angle) - second * math.sin(angle)
                output[base + pair + half] = first * math.sin(angle) + second * math.cos(angle)
    return Tensor(input_tensor.shape, tuple(output))


def _attention(queries: Tensor, keys: Tensor, values: Tensor) -> Tensor:
    sequence, query_heads, head_dimension = queries.shape
    kv_heads = keys.shape[1]
    heads_per_kv = query_heads // kv_heads
    scale = 1.0 / math.sqrt(head_dimension)
    output = [0.0] * len(queries.values)
    for query_position in range(sequence):
        for query_head in range(query_heads):
            kv_head = query_head // heads_per_kv
            scores = []
            for key_position in range(query_position + 1):
                score = 0.0
                for dimension in range(head_dimension):
                    q_index = (query_position * query_heads + query_head) * head_dimension + dimension
                    k_index = (key_position * kv_heads + kv_head) * head_dimension + dimension
                    score += queries.values[q_index] * keys.values[k_index]
                scores.append(score * scale)
            maximum = max(scores)
            exponentials = [math.exp(score - maximum) for score in scores]
            denominator = sum(exponentials)
            probabilities = [value / denominator for value in exponentials]
            for dimension in range(head_dimension):
                result = 0.0
                for key_position, probability in enumerate(probabilities):
                    value_index = (key_position * kv_heads + kv_head) * head_dimension + dimension
                    result += probability * values.values[value_index]
                output[(query_position * query_heads + query_head) * head_dimension + dimension] = result
    return Tensor(queries.shape, tuple(output))


def _decoder_block(input_tensor: Tensor, prefix: str, tensors: dict[str, Tensor], config: dict[str, str]) -> Tensor:
    hidden = int(config["hidden_size"])
    query_heads = int(config["num_heads"])
    kv_heads = int(config["num_kv_heads"])
    head_dimension = hidden // query_heads
    normalized = _rms_norm(input_tensor, tensors[prefix + "input_layernorm.weight"], float(config["norm_epsilon"]))
    queries_flat = _linear(normalized, tensors[prefix + "self_attn.q_proj.weight"])
    keys_flat = _linear(normalized, tensors[prefix + "self_attn.k_proj.weight"])
    values_flat = _linear(normalized, tensors[prefix + "self_attn.v_proj.weight"])
    queries = _rope(Tensor((input_tensor.shape[0], query_heads, head_dimension), queries_flat.values), 0, float(config["rope_theta"]))
    keys = _rope(Tensor((input_tensor.shape[0], kv_heads, head_dimension), keys_flat.values), 0, float(config["rope_theta"]))
    values = Tensor((input_tensor.shape[0], kv_heads, head_dimension), values_flat.values)
    attended = _attention(queries, keys, values)
    attention_update = _linear(Tensor((input_tensor.shape[0], hidden), attended.values), tensors[prefix + "self_attn.o_proj.weight"])
    after_attention = _add(input_tensor, attention_update)
    post_attention = _rms_norm(after_attention, tensors[prefix + "post_attention_layernorm.weight"], float(config["norm_epsilon"]))
    gate = _linear(post_attention, tensors[prefix + "mlp.gate_proj.weight"])
    up = _linear(post_attention, tensors[prefix + "mlp.up_proj.weight"])
    activated = Tensor(gate.shape, tuple((value / (1.0 + math.exp(-value))) * up.values[index] for index, value in enumerate(gate.values)))
    return _add(after_attention, _linear(activated, tensors[prefix + "mlp.down_proj.weight"]))


def _format_number(value: float) -> str:
    if math.isinf(value):
        return "-inf" if value < 0 else "inf"
    return format(value, ".17g")


def _tensor_line(name: str, tensor: Tensor) -> str:
    shape = ",".join(str(value) for value in tensor.shape)
    values = ",".join(_format_number(value) for value in tensor.values)
    return f"tensor={name}|{shape}|{values}"


def _fixture_reference(model_path: Path, token_ids: list[int]) -> str:
    config, tensors = _read_fixture(model_path)
    vocab = int(config["vocab_size"])
    hidden_size = int(config["hidden_size"])
    if not token_ids or any(token < 0 or token >= vocab for token in token_ids):
        raise ValueError("fixture tokens must be non-empty and within the vocabulary")
    embedding_values = []
    embedding_table = tensors["model.embed_tokens.weight"]
    for token in token_ids:
        embedding_values.extend(embedding_table.values[token * hidden_size : (token + 1) * hidden_size])
    embeddings = Tensor((len(token_ids), hidden_size), tuple(embedding_values))
    hidden = embeddings
    checkpoints: list[tuple[str, Tensor]] = [("model.embeddings", embeddings)]
    for layer in range(int(config["num_layers"])):
        hidden = _decoder_block(hidden, f"model.layers.{layer}.", tensors, config)
        checkpoints.append((f"model.layer.{layer}.output", hidden))
    final_hidden = _rms_norm(hidden, tensors["model.norm.weight"], float(config["norm_epsilon"]))
    logits = _linear(final_hidden, tensors["lm_head.weight"])
    last_logits = Tensor((vocab,), logits.values[-vocab:])
    checkpoints.extend((("model.final_hidden", final_hidden), ("model.logits.last", last_logits)))

    rms_input = Tensor((2, 4), (1.0, -2.0, 3.0, -4.0, 0.25, 0.5, -0.75, 1.0))
    rms_weight = Tensor((4,), (1.0, 0.5, 1.5, 2.0))
    rope_input = Tensor((2, 1, 4), (1.0, 2.0, 3.0, 4.0, -1.0, 0.5, 2.0, -3.0))
    mask = Tensor((3, 3), (1.0, -math.inf, -math.inf, 4.0, 5.0, -math.inf, 7.0, 8.0, 9.0))
    attention_queries = Tensor((2, 2, 2), (1.0, 0.0, 0.0, 1.0, 1.0, 1.0, -1.0, 0.5))
    attention_keys = Tensor((2, 1, 2), (1.0, 0.0, 0.0, 1.0))
    attention_values = Tensor((2, 1, 2), (2.0, -1.0, 4.0, 3.0))
    checkpoints.extend(
        (
            ("primitive.rmsnorm", _rms_norm(rms_input, rms_weight, 1.0e-5)),
            ("primitive.rope", _rope(rope_input, 3, 10000.0)),
            ("primitive.causal_mask", mask),
            ("primitive.attention", _attention(attention_queries, attention_keys, attention_values)),
        )
    )
    lines = [
        f"format={REFERENCE_FORMAT}",
        "source=python_stdlib_fixture_oracle",
        f"tokens={','.join(str(token) for token in token_ids)}",
        "absolute_tolerance=0.00002",
        "relative_tolerance=0.00002",
    ]
    lines.extend(_tensor_line(name, tensor) for name, tensor in checkpoints)
    return "\n".join(lines) + "\n"


def _run_fixture(args: argparse.Namespace) -> int:
    token_ids = [int(value) for value in args.tokens.split(",") if value]
    output = _fixture_reference(args.model, token_ids)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(output, encoding="utf-8", newline="\n")
    print(f"wrote fixture reference: {args.output}")
    return 0


def _run_hf(args: argparse.Namespace) -> int:
    try:
        import torch
        from transformers import AutoModelForCausalLM, AutoTokenizer
    except ImportError as error:
        raise SystemExit("HF mode requires optional torch and transformers packages") from error
    prompts = list(args.prompt)
    if args.prompt_file:
        prompts.extend(line for line in args.prompt_file.read_text(encoding="utf-8").splitlines() if line)
    if not prompts:
        raise SystemExit("HF mode requires --prompt or --prompt-file")
    if args.allow_download:
        print("WARNING: --allow-download permits Transformers to fetch model artifacts")
    local_only = not args.allow_download
    tokenizer = AutoTokenizer.from_pretrained(args.model, local_files_only=local_only)
    dtype = {"float32": torch.float32, "float16": torch.float16, "bfloat16": torch.bfloat16}[args.dtype]
    model = AutoModelForCausalLM.from_pretrained(args.model, local_files_only=local_only, torch_dtype=dtype)
    model.eval()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    requested_layers = None if args.layers == "all" else [int(value) for value in args.layers.split(",")]
    metadata = {
        "format": "tinyserve_hf_reference_v1",
        "model": args.model,
        "dtype": args.dtype,
        "local_files_only": local_only,
        "layers": args.layers,
        "logits": args.logits,
        "prompt_count": len(prompts),
    }
    (args.output_dir / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")
    for prompt_index, prompt in enumerate(prompts):
        inputs = tokenizer(prompt, return_tensors="pt")
        with torch.no_grad():
            outputs = model(**inputs, output_hidden_states=True, use_cache=False, return_dict=True)
        hidden_states = outputs.hidden_states
        if hidden_states is None:
            raise RuntimeError("model did not return hidden states")
        selected = range(len(hidden_states) - 1) if requested_layers is None else requested_layers
        payload = {
            "prompt": prompt,
            "input_ids": inputs["input_ids"].tolist(),
            "embeddings": hidden_states[0].float().cpu().tolist(),
            "layers": {str(layer): hidden_states[layer + 1].float().cpu().tolist() for layer in selected},
            "logits": (outputs.logits if args.logits == "all" else outputs.logits[:, -1, :]).float().cpu().tolist(),
        }
        (args.output_dir / f"prompt_{prompt_index:03d}.json").write_text(json.dumps(payload) + "\n", encoding="utf-8")
    print(f"wrote {len(prompts)} local HF reference dump(s): {args.output_dir}")
    return 0


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="mode", required=True)
    fixture = subparsers.add_parser("fixture", help="dump a deterministic stdlib fixture reference")
    fixture.add_argument("--model", required=True, type=Path)
    fixture.add_argument("--tokens", required=True, help="comma-separated token IDs")
    fixture.add_argument("--output", required=True, type=Path)
    fixture.set_defaults(run=_run_fixture)

    hf = subparsers.add_parser("hf", help="dump references using optional local Transformers")
    hf.add_argument("--model", required=True, help="local directory or model ID")
    hf.add_argument("--prompt", action="append", default=[])
    hf.add_argument("--prompt-file", type=Path)
    hf.add_argument("--output-dir", required=True, type=Path)
    hf.add_argument("--layers", default="all", help="all or comma-separated zero-based layer IDs")
    hf.add_argument("--logits", choices=("last", "all"), default="last")
    hf.add_argument("--dtype", choices=("float32", "float16", "bfloat16"), default="float32")
    hf.add_argument("--allow-download", action="store_true", help="explicitly permit network model downloads")
    hf.set_defaults(run=_run_hf)
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    return args.run(args)


if __name__ == "__main__":
    raise SystemExit(main())
