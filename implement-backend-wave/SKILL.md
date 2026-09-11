---
name: implement-backend-wave
description: Implement exactly one backend wave from `.tasks/backend/IMPLEMENTATION-ORDER.md`, including reading the selected wave stories, applying only that wave's code changes, running wave-level tests, updating tracker status, and validating closeout. Use when the user asks to implement backend wave N or resume a specific backend implementation session.
---

# implement-backend-wave

Implement one backend wave at a time from the generated backend planning set.

## When to Apply

- The user asks to implement a specific backend wave
- The user asks to resume a paused backend wave session
- The repo has `.tasks/backend` planning artifacts and the work should follow them directly

## Required Inputs

- The requested wave number
- `AGENTS.md`
- `.tasks/backend/IMPLEMENTATION-ORDER.md`
- `.tasks/backend/TRACKER.md`
- `.tasks/backend/technology-stack-recommendations.md`
- `docs/tech-stack/backend-technology-stack.md`
- `.tasks/backend/validate.sh`
- `.tasks/backend/validate.ps1` when running on Windows and Bash is unavailable
- The story files listed in the requested wave
- `docs/api-spec.md` when present, and `.tasks/API-SPEC.md`, when the selected stories touch API contracts. `docs/api-spec.md` is canonical: it is the runtime arbiter between the frontend and backend tracks, so implement to it exactly. If the two disagree, the published api-spec wins and the working copy is stale.

## Workflow

1. Resolve the requested wave in `.tasks/backend/IMPLEMENTATION-ORDER.md` and collect its story IDs in listed order.
2. Read `AGENTS.md`, then read the wave stories, the backend stack references, and the API spec sections named by those stories.
3. Try to resolve implementation details from local planning artifacts first: the selected story files, `.tasks/backend/IMPLEMENTATION-ORDER.md`, `.tasks/backend/TRACKER.md`, `docs/api-spec.md` (canonical) and `.tasks/API-SPEC.md`, `.tasks/backend/technology-stack-recommendations.md`, and `docs/tech-stack/backend-technology-stack.md`.
4. Confirm prerequisites. If an earlier dependency is incomplete, stop and report the blocker instead of skipping forward.
5. If a critical technical detail is still missing or likely outdated after reading local docs, use Exa as a fallback only, and only against primary sources.
6. Implement only the selected wave.
7. Run the narrowest useful tests for each story, then a wave-level validation pass.
8. Update `.tasks/backend/TRACKER.md` for the wave stories only.
9. Run `.tasks/backend/validate.sh`. On Windows, if Bash cannot launch because the environment blocks WSL/Git Bash, run `.tasks/backend/validate.ps1` instead and report that substitution.
10. Finish with a concise handoff covering completed stories, tests run, assumptions, risks, exact files changed, and any Exa usage that informed the implementation.

## Locked Rules

- Treat the story files plus `.tasks/API-SPEC.md` as the implementation contract when upstream documents drift.
- Treat local planning docs as authoritative for product behavior, API contracts, validation rules, and schema semantics.
- Do not start work from the next wave, even if nearby files make it tempting.
- Only mutate files needed for the selected wave.
- Do not update `.tasks/API-SPEC.md` unless the current wave truly changes an API contract.
- Do not mark a story done unless its required CT coverage and validation are in place.
- Prefer test-first implementation for logic-heavy stories when the story calls for it.
- Use Exa only for technical implementation guidance, never to redefine local story scope or product behavior.
- Use Exa only when local docs do not answer an implementation-critical question.
- When using Exa, rely only on primary sources such as official vendor, framework, or upstream project documentation.
- If external docs conflict with local planning artifacts, do not silently adopt the external behavior; report the conflict and continue to honor the local contract unless the user asks to reopen planning.
- If the wave is `0` and no backend workspace exists yet, create the minimal backend shell needed for that wave before implementing story details.

## External Research Fallback

Use Exa only after exhausting the local repo docs named above.

Good uses:

- current FastAPI, SQLAlchemy, Alembic, Azure, LangGraph, or similar framework usage details
- vendor configuration details that are likely to drift over time
- official examples needed to unblock a test, migration, integration, or deployment story

Do not use Exa for:

- deciding API behavior, validation rules, or schema semantics already defined locally
- resolving BSD, FSD, or story contradictions by outside guesswork
- broad research passes at the start of every wave
- replacing local architecture or implementation-order decisions

Preferred pattern:

1. Try local docs first.
2. If blocked, use Exa with primary sources only.
3. Pull the minimum guidance needed.
4. Continue implementing to the local story contract.
5. Mention Exa usage in the final handoff, including why it was needed and which primary sources informed the implementation.

## Wave 0 Rule

If the selected wave is `0`, start by checking whether `backend/` exists. If it does not, create only the smallest Python backend workspace needed for the Wave 0 stories, including:

- app package bootstrap
- database base and async session setup
- Alembic scaffold
- unit, migration, and integration test folders

Do not add product endpoints in Wave 0 unless a Wave 0 story explicitly requires them.

## Closeout Checklist

- All selected wave stories implemented
- Story-level tests run
- Wave-level validation run
- Tracker updated for the selected wave
- `.tasks/backend/validate.sh` passing, or `.tasks/backend/validate.ps1` passing on Windows when Bash is unavailable
- No later-wave work started

## Output Format

End with:

- A short implementation summary
- Tests and validation run
- Any assumptions or follow-up risks
- Exact files changed
