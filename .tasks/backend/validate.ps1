$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "../..")
Set-Location $root

$requiredFiles = @(
  "AGENTS.md",
  ".tasks/backend/IMPLEMENTATION-ORDER.md",
  ".tasks/backend/TRACKER.md",
  ".tasks/backend/technology-stack-recommendations.md",
  "docs/tech-stack/backend-technology-stack.md",
  "docs/api-spec.md",
  ".tasks/API-SPEC.md",
  "tinyserve_implementation_waves.md",
  "tinyserve_codex_execution_prompts.md"
)

foreach ($file in $requiredFiles) {
  if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
    Write-Error "Missing required file: $file"
  }
}

$storyFiles = @(
  ".tasks/backend/stories/BE-W0-S01-project-foundation.md",
  ".tasks/backend/stories/BE-W1-S01-model-boundary.md",
  ".tasks/backend/stories/BE-W2-S01-cpu-math-core.md",
  ".tasks/backend/stories/BE-W3-S01-cpu-generation-cli.md",
  ".tasks/backend/stories/BE-W4-S01-correctness-harness.md",
  ".tasks/backend/stories/BE-W5-S01-kv-cache-minimum-version.md",
  ".tasks/backend/stories/BE-W6-S01-optional-cuda-track.md",
  ".tasks/backend/stories/BE-W7-S01-quantization-serving.md",
  ".tasks/backend/stories/BE-W8-S01-final-integration.md"
)

foreach ($story in $storyFiles) {
  if (-not (Test-Path -LiteralPath $story -PathType Leaf)) {
    Write-Error "Missing story file: $story"
  }
}

Write-Host "TinyServe planning validation passed."
