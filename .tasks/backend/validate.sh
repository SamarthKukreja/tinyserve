#!/usr/bin/env sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
cd "$ROOT_DIR"

required_files="
AGENTS.md
.tasks/backend/IMPLEMENTATION-ORDER.md
.tasks/backend/TRACKER.md
.tasks/backend/technology-stack-recommendations.md
docs/tech-stack/backend-technology-stack.md
docs/api-spec.md
.tasks/API-SPEC.md
tinyserve_implementation_waves.md
tinyserve_codex_execution_prompts.md
"

for file in $required_files; do
  if [ ! -f "$file" ]; then
    echo "Missing required file: $file"
    exit 1
  fi
done

for story in \
  .tasks/backend/stories/BE-W0-S01-project-foundation.md \
  .tasks/backend/stories/BE-W1-S01-model-boundary.md \
  .tasks/backend/stories/BE-W2-S01-cpu-math-core.md \
  .tasks/backend/stories/BE-W3-S01-cpu-generation-cli.md \
  .tasks/backend/stories/BE-W4-S01-correctness-harness.md \
  .tasks/backend/stories/BE-W5-S01-kv-cache-minimum-version.md \
  .tasks/backend/stories/BE-W6-S01-optional-cuda-track.md \
  .tasks/backend/stories/BE-W7-S01-quantization-serving.md \
  .tasks/backend/stories/BE-W8-S01-final-integration.md
do
  if [ ! -f "$story" ]; then
    echo "Missing story file: $story"
    exit 1
  fi
done

echo "TinyServe planning validation passed."

