#!/usr/bin/env python3
"""Validate TinyServe's saved fixture reference format and required checkpoints."""

from __future__ import annotations

import argparse
import math
from pathlib import Path


REQUIRED = {
    "model.embeddings",
    "model.layer.0.output",
    "model.final_hidden",
    "model.logits.last",
    "primitive.rmsnorm",
    "primitive.rope",
    "primitive.causal_mask",
    "primitive.attention",
}


def validate(path: Path) -> None:
    fields: dict[str, str] = {}
    tensors: set[str] = set()
    for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if "=" not in line:
            raise ValueError(f"line {line_number}: expected key=value")
        key, value = line.split("=", 1)
        if key != "tensor":
            fields[key] = value
            continue
        name, shape_text, values_text = value.split("|", 2)
        shape = tuple(int(part) for part in shape_text.split(","))
        values = tuple(float(part) for part in values_text.split(","))
        expected = math.prod(shape)
        if len(values) != expected:
            raise ValueError(f"line {line_number}: {name} has {len(values)} values, expected {expected}")
        if name in tensors:
            raise ValueError(f"line {line_number}: duplicate tensor {name}")
        tensors.add(name)
    if fields.get("format") != "tinyserve_reference_v1":
        raise ValueError("unsupported or missing reference format")
    if fields.get("source") != "python_stdlib_fixture_oracle":
        raise ValueError("fixture reference must identify the independent stdlib oracle")
    for tolerance in ("absolute_tolerance", "relative_tolerance"):
        if not (0.0 < float(fields.get(tolerance, "0")) <= 0.001):
            raise ValueError(f"invalid {tolerance}")
    missing = REQUIRED - tensors
    if missing:
        raise ValueError(f"missing checkpoints: {', '.join(sorted(missing))}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", required=True, type=Path)
    args = parser.parse_args()
    validate(args.reference)
    print(f"valid reference: {args.reference}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
