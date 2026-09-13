# Usage Report — Final Full-Dataset Run

Run date (UTC): 2026-09-13T11:16:14.614774+00:00
Requests: 250
Elapsed: 3.3s

Engine: deterministic Python stdlib (no LLM calls for core decisions). Image amounts recovered via manual inspection mapping embedded in `code/main.py:IMAGE_AMOUNTS` (16 PNGs); no vision API calls.

## Model usage

| Provider | Model | Calls | Input tokens | Output tokens | Total tokens | Est. cost |
|---|---|---|---|---|---|---|
| none (deterministic) | n/a | 0 | 0 | 0 | 0 | $0.00 |

## Totals

- Total input tokens: 0
- Total output tokens: 0
- Total tokens: 0
- Average tokens per request: 0.0
- Estimated total cost: $0.00
- Estimated per-request cost: $0.00

## Notes

- Token counts correspond to the final run that produced root `output.csv`.
- No API keys or credentials used. No network calls.
- If a vision/LLM assist was used during development for message translation, it is not part of the final run and is excluded here per spec (final run only).
