# TinyServe Backend Technology Stack Recommendations

TinyServe should be built as a small systems project, not a full production model server.

Recommended core stack:

- Language: C++17 or C++20
- Build: CMake
- Tests: Catch2, GoogleTest, or a lightweight local test harness
- Scripts: Python 3 for offline conversion, reference dumps, and benchmark result processing
- Benchmarks: reproducible command-line scripts with saved JSON/CSV outputs
- Documentation: Markdown reports checked into `docs/`

Optional extensions:

- CUDA: optional CMake build gate, never required for CPU-only validation
- BLAS/cuBLAS: allowed for microbenchmarks if documented honestly
- HTTP server: lightweight C++ HTTP library or minimal Python wrapper around the CLI/runtime
- Quantization: custom INT8 fixture path before any real-model claims

Avoid:

- Overclaiming production readiness
- Hiding framework dependencies
- Large opaque dependencies that make the implementation look like a wrapper
- Benchmarks without saved commands, machine details, and raw outputs

