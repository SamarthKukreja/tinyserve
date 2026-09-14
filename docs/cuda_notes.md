# Optional CUDA Track

Wave 6 establishes a clean CUDA build and hardware gate without making CUDA a dependency of TinyServe's minimum strong CPU version. CUDA kernels and performance claims are intentionally deferred on this machine because neither a usable toolkit nor a CUDA device could be validated.

## Local gate result

Detection performed on 2026-08-17:

| Probe | Result |
| --- | --- |
| `Get-Command nvcc` | Not found |
| `Get-Command nvidia-smi` | Not found |
| `CUDA_PATH` | Not set |
| `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA` | No installation found |
| `Get-CimInstance Win32_VideoController` | Access denied by the environment |
| CMake `check_language(CUDA)` | CUDA compiler not found |

The missing compiler/toolkit is decisive: CUDA code cannot be built or numerically validated here. No dependency installation was attempted, no cuBLAS approval was requested, and no GPU benchmark numbers were created.

## Build gate

CUDA probing is off by default, so the normal commands remain CPU-only:

```bash
cmake -S . -B build -DTINYSERVE_ENABLE_CUDA=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

An operator with an existing CUDA installation can request the optional track:

```bash
cmake -S . -B build-cuda \
  -DTINYSERVE_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86
cmake --build build-cuda
ctest --test-dir build-cuda --output-on-failure
```

Use the architecture value for the actual target GPU; `86` above is only a syntax example, not a recommendation for this uninspected machine. CMake 3.18 or newer is recommended for `CMAKE_CUDA_ARCHITECTURES`. With older CUDA-capable CMake versions, architecture compiler flags must be supplied explicitly and recorded with any result.

When requested, CMake checks for a CUDA compiler and `CUDAToolkit`. If either is absent, it emits a warning, omits all CUDA targets, writes `tinyserve_cuda_status.txt`, and continues configuring the CPU runtime. If both are present, it exposes `tinyserve_cuda_check`; that executable records runtime/driver versions and visible device properties. Its CTest case uses skip code 77 when the toolkit is usable but no CUDA device is visible.

The locally validated unavailable-gate command was:

```powershell
cmake -S . -B build-cuda-gate -G "NMake Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DTINYSERVE_ENABLE_CUDA=ON `
  -DBUILD_TESTING=OFF `
  -DTINYSERVE_BUILD_BENCHMARKS=OFF
cmake --build build-cuda-gate
```

It completed successfully with `compiler=not-found`, `toolkit_found=OFF`, and `targets_available=OFF`, while building the CPU executable.

## Kernel and benchmark status

| Planned item | Status | Reason |
| --- | --- | --- |
| CUDA availability probe | Implemented; compilation/runtime execution gated | Requires existing toolkit; no toolkit locally |
| CUDA RMSNorm | Deferred with reason | Cannot compile or compare with CPU RMSNorm |
| CUDA RoPE | Deferred with reason | Cannot compile or compare deterministic positions/layout |
| Naive CUDA GEMV | Deferred with reason | Cannot compile or measure kernels |
| cuBLAS GEMV baseline | Deferred with reason | Toolkit not installed; no dependency or cuBLAS use approved |
| CUDA inference hooks | Not implemented | No validated kernels to wire safely |
| CUDA benchmark output | Not created | No GPU path was executable; inventing results is prohibited |

If work resumes on validated hardware, the required order remains RMSNorm correctness, RoPE correctness, naive decode-oriented GEMV, then an already-provided cuBLAS comparison. Each kernel must compare with the CPU implementation, save raw benchmark outputs, and record GPU, compute capability, toolkit, driver, architecture flags, dtype, shapes, warmups, repeats, and timing method.

## Bottleneck hypotheses, not measurements

The following guide later profiling but are not observed results:

- RMSNorm and RoPE perform little arithmetic per element and may be limited by global-memory traffic or kernel-launch overhead at small shapes.
- Batch-one decode GEMV has low weight reuse and is commonly analyzed as memory-bandwidth-sensitive; TinyServe must measure achieved bandwidth before making that claim for a concrete kernel.
- Separate microkernels can lose time to launches and intermediate memory traffic; production engines use fusion, tuned libraries, scheduling, and batching that this educational runtime does not provide.
- A microkernel speedup would not establish end-to-end generation acceleration until model execution is actually wired and benchmarked.

TinyServe therefore continues to report `cpu` behavior only. CUDA, cuBLAS performance, GPU memory use, and end-to-end acceleration remain unsupported and unclaimed.
