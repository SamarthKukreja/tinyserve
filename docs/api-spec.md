# TinyServe API Spec

This API spec is canonical for the optional serving layer in Wave 7.

TinyServe is not expected to expose an API before Wave 7. Earlier waves should not implement server endpoints.

## GET `/health`

Purpose:

- Report whether the server process is alive.

Response `200`:

```json
{
  "status": "ok",
  "runtime": "cpu",
  "model_loaded": true
}
```

Notes:

- `runtime` may be `cpu`, `cuda`, or `unknown`.
- `model_loaded` must reflect actual runtime state.

## POST `/generate`

Purpose:

- Generate text from a prompt using the implemented TinyServe runtime.

Request:

```json
{
  "prompt": "Hello",
  "max_new_tokens": 16,
  "temperature": 0.0,
  "top_k": 1,
  "top_p": 1.0
}
```

Response `200`:

```json
{
  "text": "Hello ...",
  "tokens_generated": 16,
  "runtime": "cpu",
  "latency_ms": 12.3
}
```

Validation rules:

- `prompt` is required and must be a string.
- `max_new_tokens` must be positive and bounded by a documented server limit.
- Sampling fields must use the same semantics as the CLI.
- The server must not claim streaming support unless streaming is implemented and tested.

## GET `/metrics`

Purpose:

- Return simple local process metrics when implemented.

Response `200`:

```json
{
  "requests_total": 0,
  "tokens_generated_total": 0,
  "last_latency_ms": null
}
```

Notes:

- This endpoint is optional for Wave 7.
- If not implemented, docs and load-test reports must not mention it as available.

