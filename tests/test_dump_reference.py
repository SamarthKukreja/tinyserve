#!/usr/bin/env python3
"""Exercise the no-download reference tool and lock the saved fixture output."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def run(command: list[str]) -> None:
    subprocess.run(command, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dumper", required=True, type=Path)
    parser.add_argument("--checker", required=True, type=Path)
    parser.add_argument("--model", required=True, type=Path)
    parser.add_argument("--saved-reference", required=True, type=Path)
    parser.add_argument("--work-dir", required=True, type=Path)
    args = parser.parse_args()
    args.work_dir.mkdir(parents=True, exist_ok=True)
    outputs = [args.work_dir / "reference-a.txt", args.work_dir / "reference-b.txt"]
    for output in outputs:
        run(
            [
                sys.executable,
                str(args.dumper),
                "fixture",
                "--model",
                str(args.model),
                "--tokens",
                "65,66",
                "--output",
                str(output),
            ]
        )
        run([sys.executable, str(args.checker), "--reference", str(output)])
    first = outputs[0].read_bytes()
    if first != outputs[1].read_bytes():
        raise AssertionError("two fixture oracle runs were not byte-identical")
    if first != args.saved_reference.read_bytes():
        raise AssertionError(
            "saved fixture reference is stale; regenerate it with tools/dump_reference.py"
        )
    print("fixture reference is deterministic, valid, and current")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
