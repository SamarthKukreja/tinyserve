#!/usr/bin/env python3
"""Write deterministic fixture files for the TinyServe weight format.

Real Hugging Face or safetensors conversion is intentionally not implemented in
Wave 1. This script uses only the Python standard library.
"""

from __future__ import annotations

import argparse
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence


MAGIC = b"TSERVE01"
FORMAT_VERSION = 1
FLOAT32 = 1


@dataclass(frozen=True)
class Tensor:
    name: str
    shape: tuple[int, ...]
    values: tuple[float, ...]
    dtype: int = FLOAT32


DUMMY_CONFIG = """vocab_size=256
hidden_size=4
intermediate_size=8
num_layers=1
num_heads=2
num_kv_heads=1
rope_theta=10000
norm_epsilon=0.00001
max_sequence_length=16
dtype=float32
"""


def _product(values: Sequence[int]) -> int:
    result = 1
    for value in values:
        if value <= 0:
            raise ValueError("tensor dimensions must be positive")
        result *= value
    return result


def _series(count: int, offset: int) -> tuple[float, ...]:
    return tuple(((index + offset) % 17 - 8) * 0.125 for index in range(count))


def _identity(rows: int, columns: int) -> tuple[float, ...]:
    return tuple(
        1.0 if row == column else 0.0
        for row in range(rows)
        for column in range(columns)
    )


def dummy_tensors() -> tuple[Tensor, ...]:
    """Return a complete one-layer deterministic float32 fixture model."""

    tensors = (
        Tensor("lm_head.weight", (256, 4), _series(1024, 3)),
        Tensor("model.embed_tokens.weight", (256, 4), _series(1024, 0)),
        Tensor("model.layers.0.input_layernorm.weight", (4,), (1.0, 1.125, 0.875, 1.25)),
        Tensor("model.layers.0.mlp.down_proj.weight", (4, 8), _series(32, 7)),
        Tensor("model.layers.0.mlp.gate_proj.weight", (8, 4), _series(32, 5)),
        Tensor("model.layers.0.mlp.up_proj.weight", (8, 4), _series(32, 9)),
        Tensor("model.layers.0.post_attention_layernorm.weight", (4,), (1.0, 0.875, 1.125, 1.0)),
        Tensor("model.layers.0.self_attn.k_proj.weight", (2, 4), _identity(2, 4)),
        Tensor("model.layers.0.self_attn.o_proj.weight", (4, 4), _identity(4, 4)),
        Tensor("model.layers.0.self_attn.q_proj.weight", (4, 4), _identity(4, 4)),
        Tensor("model.layers.0.self_attn.v_proj.weight", (2, 4), _identity(2, 4)),
        Tensor("model.norm.weight", (4,), (1.0, 1.0, 1.0, 1.0)),
    )
    return tuple(sorted(tensors, key=lambda tensor: tensor.name))


def encode_weight_file(config_text: str, tensors: Iterable[Tensor]) -> bytes:
    config_bytes = config_text.encode("utf-8")
    ordered = tuple(sorted(tensors, key=lambda tensor: tensor.name))
    if len({tensor.name for tensor in ordered}) != len(ordered):
        raise ValueError("tensor names must be unique")

    output = bytearray()
    output += MAGIC
    output += struct.pack("<III", FORMAT_VERSION, len(config_bytes), len(ordered))
    output += config_bytes

    for tensor in ordered:
        if tensor.dtype != FLOAT32:
            raise ValueError(f"unsupported fixture dtype code {tensor.dtype}")
        if not tensor.name or not tensor.shape:
            raise ValueError("tensor name and shape must be non-empty")
        if _product(tensor.shape) != len(tensor.values):
            raise ValueError(f"tensor {tensor.name!r} value count does not match its shape")
        name = tensor.name.encode("utf-8")
        body = struct.pack(f"<{len(tensor.values)}f", *tensor.values)
        output += struct.pack("<IIIQ", len(name), tensor.dtype, len(tensor.shape), len(body))
        output += name
        output += struct.pack(f"<{len(tensor.shape)}Q", *tensor.shape)
        output += body
    return bytes(output)


def write_dummy_fixture(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(encode_weight_file(DUMMY_CONFIG, dummy_tensors()))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--dummy-output",
        required=True,
        type=Path,
        help="path for a deterministic, synthetic .tserve fixture",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    write_dummy_fixture(args.dummy_output)
    print(f"wrote deterministic TinyServe dummy fixture: {args.dummy_output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
