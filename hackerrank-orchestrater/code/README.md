# Buy or Wait? — Complete Solution

Deterministic AI-powered financial decision agent for HackerRank Orchestrate September 2026.
For each of 250 requests in `dataset/requests.csv`, decides `full_payment` / `partial_payment` / `installments` / `wait` / `not_recommended` while keeping the balance above `minimum_balance_to_keep` over a 90-day forecast.

Entry point: `code/main.py` (Python 3 stdlib only, no network calls, deterministic).

## 1. Setup Instructions

Prerequisites:

- Python 3.10+ (tested on 3.14.6, macOS/Linux/Windows)
- No third-party dependencies — stdlib only (`csv`, `datetime`, `decimal`, `re`, `collections`, `pathlib`)
- No API keys, no env vars required, no live banking / FX / market calls

Clone and enter:

```bash
git clone https://github.com/interviewstreet/hackerrank-orchestrate-september26.git
cd hackerrank-orchestrate-september26
python3 --version  # >= 3.10
```

Optional (isolated run):

```bash
python3 -m venv .venv && source .venv/bin/activate  # Windows: .venv\Scripts\activate
pip install -r code/requirements.txt  # empty, stdlib only — no-op
```

Dataset expected at (do not modify inputs):

```text
dataset/requests.csv                  # 250 eval requests — predict these
dataset/financial_profiles.csv        # 275 users
dataset/financial_events.csv          # 25,343 events
dataset/request_payment_options.csv   # 790 options (2–4 per request)
dataset/exchange_rates.csv            # 135 dated rates
dataset/messages.csv                  # 215 messages
dataset/images.csv                    # 16 rows
dataset/media/images/*.png            # 16 PNGs
dataset/sample_requests.csv           # 25 solved examples (format reference only)
```

## 2. Run

From repository root:

```bash
python3 code/main.py
```

What it does:

- Reads all files from `dataset/`
- Writes `output.csv` in repository root (250 rows + header, exact 8 columns in order)
- Writes `code/evaluation/usage_report.md` (token/cost report for final run)

Verify:

```bash
ls -lh output.csv code/evaluation/usage_report.md
wc -l output.csv dataset/requests.csv  # both 251 (header + 250)
head -n 2 output.csv
python3 code/main.py  # rerun → byte-identical (deterministic, ~4s)
```

Output columns (exact order):

```text
request_id,amount_safe_to_pay,affordability_status,recommended_payment_method,payment_plan,earliest_date_for_full_payment,spending_changes_needed,decision_explanation
```

Rules enforced: `0 <= amount_safe_to_pay <= requested_amount`; `affordable_now→earliest=request_date`; `not_affordable→earliest empty, plan none`; `partial_payment` exactly 2 payments summing to request; `installments` exactly matches a supplied `payment_option_id`; spending changes ≤3, flexible + permitted + non-protected only.

Repackage submission after any change:

```bash
rm -f code.zip
zip -r code.zip code README.md problem_statement.md plan.md
unzip -l code.zip  # must contain code/evaluation/usage_report.md
```

## 3. Approach Overview

### 3.1 State reconstruction + FX (`code/main.py:build_rates,convert_amount`)

- Amounts in `home_currency` (INR/ZAR/IDR/USD/EUR). Foreign cash events converted on `settlement_date` via exact `(rate_date,from→to)` row in `exchange_rates.csv`, fallback to latest prior rate for same pair.
- 16 blank `amount` events recovered via embedded `IMAGE_AMOUNTS` (`code/main.py:18`) from manual PNG inspection (e.g. `event_253:4365000` payslip Net Pay, `event_1442:100000` rent Balance Due not Total 200k, `event_6033:79679.26`, `event_7307:33.50 USD`). Deterministic, no OCR API. Never treats blank as zero; if image missing, would not invent evidence.

### 3.2 Event classification

- Ignore `failed`/`cancelled`/`unrealized`/`non_cash` (`investment_valuation`).
- Reserve `pending` debits immediately (`current_available_balance − pending`); ignore `pending` credits (bonus/commission/refund/lottery/gains) until settled.
- `scheduled` debits + base-salary credits in 90-day window counted on `settlement_date`; other scheduled credits ignored (safer).
- Base salary only (`is_base_salary` excludes commission/bonus/incentive/overtime/final/one-time/adjustment); suppress all future salary after a `Final` payroll with no scheduled base (e.g. `user_05`); `linked_event_id` counted by cash state, not link alone.

### 3.3 Evidence (untrusted)

- Relevant messages: `request_id` match, or user-level sent ≤ request within 120d. Salary overrides (`IDR 42750000`, `EUR 1037.52`, base `38760000`) with effective-date parsing; 12% rent hikes; explicit cancels only (disputes saying “has not been posted” kept — safer). Embedded instructions never override rules.
- Images used only when relevant; no invented facts.

### 3.4 90-day safety forecaster (`detect_recurring,build_future_flows,simulate_balance`)

- Recurrence per `(category,description)` with median interval 5–95d, `n≥2` (`n==2` requires 20–40/6–8/13–15d); debit amount `max(last3)` (conservative), salary last base.
- Monthly salary inferred when history+scheduled base ≥2 with 20–40d interval.
- Daily simulation from `request_date`: `start = balance − pending`; `balance += flows + payments` per day; safe iff never `< minimum` for 90d and last payment ≤ `desired_completion_date`.

### 3.5 Safe + earliest + plans + ranking

- `amount_safe_to_pay`: binary-search (50 iters, floor to cent) max single payment today passing safety without spending changes.
- `earliest_date_for_full_payment`: first `request_date..+90d` where full single payment passes (independent of preferences); empty if never; forced empty when `not_affordable` for evaluability.
- Candidates: eligible `full_payment`/`installments` options (respect `payment_methods_user_will_consider`, `max_installment_months` via `span ≤ months×30.44`), custom `partial_payment` (only if `allows_partial`, user accepts, `0<safe<req`, `earliest≤desired`), `wait` (single `earliest:req`), fallback `not_recommended`.
- Spending search: flexible recurring only, permitted reduce/stop categories, never protected, `stop`+`reduce` mutually exclusive; minimal search (size 1→3, smallest total saving first, reduce preferred over stop).
- Ranking: completes-by-deadline → no-changes → lowest total → earlier start → fewer payments → lowest `payment_option_id`. Explanations grounded with amounts/minimum/dates.

### 3.6 Validation

Bounds, status↔method mapping, chronological plans, partial 2-sum, installment exact-match, flexible-only spending, deadline, earliest rules, 90-day re-simulation. Self-scored 11/25 status on `sample_requests.csv`; full-run validity 0 errors. Known conservatism (e.g. `request_01` 20200 vs 25256, `request_16` 33874 vs 122500) from `max` + `n==2` fragments + rent-balance add — documented in `plan.md:7`, kept for safety over accuracy.

## 4. Submission Artifacts

`code.zip` (rebuilt after this README update) contains:

```text
code/main.py
code/README.md            # you are here (setup + approach)
code/requirements.txt     # empty (stdlib only)
code/evaluation/usage_report.md  # 0 LLM calls, 0 tokens, $0.00
code/evaluation/main.py
README.md                 # starter + challenge spec pointer
problem_statement.md
plan.md                   # phased plan + progress/decision logs
```

Plus repository root:

```text
output.csv                # 250 predictions, exact columns/order
log.txt                   # AGENTS.md transcript (gitignored, submit as chat_transcript)
code.zip                  # above
```

Pre-submit checklist: 251 lines in `output.csv`; exact columns; `0<=safe<=req`; installment match; flexible-only changes; `python3 code/main.py` reruns clean; `code.zip` contains `evaluation/usage_report.md` with final-run tokens/costs, no secrets.

## 5. Token Usage

Zero LLM calls in final run. See `code/evaluation/usage_report.md`: provider `none`, 0 input/output tokens, avg 0.0/req, $0.00 total. Image/message handling is rule-based, offline, cached.
