#!/usr/bin/env python3
"""Render and verify Wave 5 benchmark documentation from saved CSV results."""

from __future__ import annotations

import argparse
import csv
import hashlib
from pathlib import Path


START = "<!-- KV_BENCHMARK_START -->"
END = "<!-- KV_BENCHMARK_END -->"


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def validate(summary: list[dict[str, str]], raw: list[dict[str, str]]) -> dict[str, dict[str, str]]:
    by_mode = {row["mode"]: row for row in summary}
    if set(by_mode) != {"no-cache", "kv-cache"} or len(summary) != 2:
        raise ValueError("summary must contain exactly no-cache and kv-cache rows")
    required_metadata = (
        "timestamp_utc",
        "source_revision",
        "build_type",
        "compiler",
        "os",
        "cpu",
        "ram_bytes",
        "thread_settings",
        "batch_size",
        "seed",
        "sampling",
        "model",
        "model_checksum",
        "precision",
        "command",
    )
    for row in summary:
        for field in required_metadata:
            if not row.get(field):
                raise ValueError(f"{row['mode']} is missing {field}")
        if int(row["repeats"]) < 3 or int(row["warmups"]) < 1:
            raise ValueError("benchmark must retain at least three repeats and one warmup")
        if float(row["median_ttft_ms"]) <= 0 or float(row["decode_tokens_per_sec"]) <= 0:
            raise ValueError("benchmark timing values must be positive")
    stable_fields = (
        "timestamp_utc",
        "source_revision",
        "build_type",
        "compiler",
        "os",
        "cpu",
        "ram_bytes",
        "thread_settings",
        "batch_size",
        "seed",
        "sampling",
        "model",
        "model_checksum",
        "precision",
        "prompt_tokens",
        "generated_tokens",
        "warmups",
        "repeats",
        "inner_iterations",
    )
    for field in stable_fields:
        if by_mode["no-cache"][field] != by_mode["kv-cache"][field]:
            raise ValueError(f"comparison changed {field}")
    expected_raw = int(by_mode["no-cache"]["repeats"]) * 2
    if len(raw) != expected_raw:
        raise ValueError(f"raw sample count is {len(raw)}, expected {expected_raw}")
    if {row["mode"] for row in raw} != {"no-cache", "kv-cache"}:
        raise ValueError("raw samples do not cover both modes")
    return by_mode


def result_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def table(by_mode: dict[str, dict[str, str]]) -> str:
    lines = [
        "| Mode | Median TTFT (ms) | Median decode (ms) | Decode tok/s | Persistent KV bytes |",
        "| --- | ---: | ---: | ---: | ---: |",
    ]
    for mode in ("no-cache", "kv-cache"):
        row = by_mode[mode]
        lines.append(
            f"| {mode} | {row['median_ttft_ms']} | {row['median_decode_ms']} | "
            f"{row['decode_tokens_per_sec']} | {row['persistent_kv_bytes']} |"
        )
    return "\n".join(lines)


def interpretation(by_mode: dict[str, dict[str, str]]) -> str:
    no_cache = float(by_mode["no-cache"]["decode_tokens_per_sec"])
    cached = float(by_mode["kv-cache"]["decode_tokens_per_sec"])
    ratio = cached / no_cache
    if ratio >= 1.0:
        return f"For this run, cached decode throughput was {ratio:.3f}x the no-cache fixture throughput."
    return (
        f"For this run, cached decode throughput was {ratio:.3f}x the no-cache fixture throughput; "
        "cache bookkeeping outweighed avoided computation at this synthetic model size."
    )


def render_kv_doc(by_mode: dict[str, dict[str, str]], summary_hash: str, raw_hash: str) -> str:
    row = by_mode["no-cache"]
    return f"""# KV Cache Benchmark Results

This file is generated from `bench/results.csv` and `bench/raw_results.csv` by `tools/render_kv_results.py`. Do not hand-edit measured values.

## Fixture result

{table(by_mode)}

{interpretation(by_mode)} The contiguous cache reserves keys and values in `[layer][position][kv_head][head_dimension]` order. It avoids recomputing prior-token projections and decoder blocks, while trading `{by_mode['kv-cache']['persistent_kv_bytes']}` persistent bytes at the measured capacity for the fixture.

## Run metadata

- Timestamp (UTC): `{row['timestamp_utc']}`
- Source revision: `{row['source_revision']}` (the workspace is not a Git repository)
- Build/compiler: `{row['build_type']}` / `{row['compiler']}`
- Host: `{row['os']}` / `{row['cpu']}`
- Physical RAM bytes / thread settings: `{row['ram_bytes']}` / `{row['thread_settings']}`
- Model: `{row['model']}` (`{row['model_checksum']}`, `{row['precision']}`)
- Batch/seed/sampling: `{row['batch_size']}` / `{row['seed']}` / `{row['sampling']}`
- Prompt/generated/decode tokens: `{row['prompt_tokens']}` / `{row['generated_tokens']}` / `{row['decode_tokens']}`
- Warmups/repeats/inner iterations: `{row['warmups']}` / `{row['repeats']}` / `{row['inner_iterations']}`
- Process peak memory: `{row['process_peak_memory']}`; only exact persistent KV allocation is reported
- Command: `{row['command']}`
- Summary SHA-256: `{summary_hash}`
- Raw samples SHA-256: `{raw_hash}`

TTFT includes fixture prompt processing and selection of the first generated token. Decode throughput excludes that first token. Each saved sample is the per-request average of the recorded inner iterations, and the table reports the median across repeats.

## Limits

These numbers describe a one-layer, hidden-size-4 synthetic float32 fixture on one local CPU. They are useful for validating the benchmark and exposing the algorithmic tradeoff, but they are not real-model performance, an external-engine comparison, or evidence of language quality. The cached prefill path is deliberately simple and processes prompt tokens incrementally. Paged caching, batching, eviction, prefix reuse, GPU memory, and process peak-memory instrumentation remain out of scope.
"""


def render_report(by_mode: dict[str, dict[str, str]]) -> str:
    return f"""# TinyServe Minimum Strong Version Report

Status: **Wave 5 minimum strong version complete for the deterministic fixture**.

TinyServe now contains a readable C++17 CPU inference path, offline reference comparisons, deterministic generation, a contiguous single-sequence KV cache, cached/no-cache equivalence tests, and a reproducible benchmark with retained raw samples.

## Architecture

The runtime loads a strict versioned tensor file, embeds byte-fixture token IDs, applies scalar RMSNorm, split-half RoPE, grouped-query causal attention, SwiGLU decoder blocks, final normalization, and the LM head. No-cache generation recomputes the full prefix. KV-cache generation processes one new token per step and stores each layer's rotated keys and values contiguously.

## Measured fixture comparison

{table(by_mode)}

{interpretation(by_mode)} See `docs/kv_cache_results.md` and the checked-in CSV files for the exact environment, command, checksums, and raw repetitions.

## Correctness evidence

- Independent Python fixture references cover primitives, layer output, final hidden state, and full last-token logits.
- Cached incremental logits match the no-cache path within the fixture tolerance.
- Greedy cached and no-cache token sequences match exactly.
- Cache allocation, append/retrieve, capacity, geometry, reset, and repeated-run behavior are tested.

## Honest limits and next steps

Only the synthetic float32 fixture is supported and measured. A real model/tokenizer has not been approved or validated. CUDA, INT8, paged caching, batching, HTTP serving, external-engine comparisons, and production claims are absent. Optional CUDA investigation is the next planned wave and must remain CPU-build-gated.

Suggested milestone tag: `v0.3-kv-cache`. No Git tag was created.
"""


def render_readme_block(by_mode: dict[str, dict[str, str]]) -> str:
    return f"""{START}
### Measured fixture result

{table(by_mode)}

{interpretation(by_mode)} This is a tiny synthetic fixture result, not real-model performance. Full metadata and raw samples are in [KV cache results](docs/kv_cache_results.md).
{END}"""


def update_readme(text: str, block: str) -> str:
    if START not in text or END not in text:
        raise ValueError("README is missing KV benchmark generation markers")
    prefix, remainder = text.split(START, 1)
    _, suffix = remainder.split(END, 1)
    return prefix + block + suffix


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True, type=Path)
    parser.add_argument("--raw-results", required=True, type=Path)
    parser.add_argument("--kv-doc", required=True, type=Path)
    parser.add_argument("--report", required=True, type=Path)
    parser.add_argument("--readme", required=True, type=Path)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    by_mode = validate(read_rows(args.results), read_rows(args.raw_results))
    kv_doc = render_kv_doc(by_mode, result_hash(args.results), result_hash(args.raw_results))
    report = render_report(by_mode)
    block = render_readme_block(by_mode)
    readme = update_readme(args.readme.read_text(encoding="utf-8"), block)
    if args.check:
        if args.kv_doc.read_text(encoding="utf-8") != kv_doc:
            raise SystemExit("docs/kv_cache_results.md is stale")
        if args.report.read_text(encoding="utf-8") != report:
            raise SystemExit("docs/final_benchmark_report.md is stale")
        if args.readme.read_text(encoding="utf-8") != readme:
            raise SystemExit("README benchmark block is stale")
        print("KV benchmark results and generated documentation agree")
        return 0
    args.kv_doc.write_text(kv_doc, encoding="utf-8", newline="\n")
    args.report.write_text(report, encoding="utf-8", newline="\n")
    args.readme.write_text(readme, encoding="utf-8", newline="\n")
    print("rendered KV benchmark documentation from saved CSV results")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
