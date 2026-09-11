#!/usr/bin/env python3
"""Validate that dummy conversion is deterministic and structurally readable."""

from __future__ import annotations

import argparse
import hashlib
import struct
import subprocess
import sys
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", required=True, type=Path)
    parser.add_argument("--work-dir", required=True, type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    first = args.work_dir / "first.tserve"
    second = args.work_dir / "second.tserve"
    for output in (first, second):
        subprocess.run(
            [sys.executable, str(args.converter), "--dummy-output", str(output)],
            check=True,
            capture_output=True,
            text=True,
        )

    first_bytes = first.read_bytes()
    second_bytes = second.read_bytes()
    if first_bytes != second_bytes:
        raise AssertionError("dummy fixture conversion is not byte-for-byte deterministic")
    if hashlib.sha256(first_bytes).digest() != hashlib.sha256(second_bytes).digest():
        raise AssertionError("dummy fixture hashes differ")
    if first_bytes[:8] != b"TSERVE01":
        raise AssertionError("fixture magic is invalid")
    version, config_size, tensor_count = struct.unpack_from("<III", first_bytes, 8)
    if version != 1 or config_size == 0 or tensor_count != 12:
        raise AssertionError("fixture header metadata is invalid")

    print("TinyServe converter determinism test passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
