# QuantLib Python Drop-In Replacement

> **This is a pure Python reimplementation of the QuantLib C++ library.**
> The goal is full API compatibility: Python code written against this library should work identically to code written against the original QuantLib C++ library (via its Python bindings), with no C++ compilation or native extensions required.

**Target users**: Python developers who want QuantLib functionality without installing C++ bindings (`QuantLib-SWIG` / `QuantLib-Python`).

This is NOT a wrapper, NOT a simplified subset, and NOT a new API. It deliberately copies QuantLib's existing API — same class names, method signatures, and behavior.

---

## Critical Rules

These rules are non-negotiable. Every session must follow them.

1. **NEVER target `lballabio/QuantLib`** — always use `--repo itayfeldman/QuantLib` for all `gh` commands. This is a fork; the upstream is someone else's project.
2. **NEVER work directly in the main checkout** — always create a git worktree. Multiple Claude sessions run in parallel.
3. **NEVER commit to `master`** — master is production. All work goes through PRs.
4. **NEVER modify files in `upstream/`** — that's the read-only C++ reference.
5. **NEVER write tests from scratch** — transpile from the C++ test suite.

---

## Git Worktrees (Required)

Multiple Claude Code sessions run simultaneously. Each session MUST use its own git worktree to avoid corrupting other sessions' work.

```bash
# Before ANY work — create an isolated worktree
git worktree add /tmp/ql-task-<NUMBER> -b task/<NUMBER>-<desc> origin/master
cd /tmp/ql-task-<NUMBER>

# ... do all work inside the worktree ...

# After PR merges — clean up
git worktree remove /tmp/ql-task-<NUMBER>
```

Or use the Agent tool with `isolation: "worktree"` which handles this automatically.

---

## The SDLC Loop

The development lifecycle is: **Plan → Implement → Verify → Deliver → Repeat**.

```
Human creates Epic
  → Agent breaks into Stories with AGENT-META
    → Stories wired as GitHub sub-issues
      → auto-advance-deps unblocks status/ready issues
        → Agent picks up issue via /dispatch-work
          → Creates worktree + branch
            → TDD: transpile C++ tests → RED → GREEN
              → Push → Create PR with "Closes #N"
                → CI runs (lint + typecheck + test + coverage)
                  → Auto-merge on pass
                    → Issue closes → deps unblock → epic auto-closes
                      → Next issue
```

---

## Planning: GitHub Issues

All planning lives in GitHub Issues. No local files, no task lists for planning.

### Issue Hierarchy

- **Epics** (`type/epic`): Define "what" — product scope, acceptance criteria
- **Stories** (`type/story`): Define "how" — technical approach, files to create, C++ references

Link stories to epics using **GitHub sub-issues** (GraphQL `addSubIssue` mutation, NOT markdown links):

```bash
EPIC_ID=$(gh api graphql -f query='query { repository(owner: "itayfeldman", name: "QuantLib") { issue(number: N) { id } } }' --jq '.data.repository.issue.id')
STORY_ID=$(gh api graphql -f query='query { repository(owner: "itayfeldman", name: "QuantLib") { issue(number: M) { id } } }' --jq '.data.repository.issue.id')
gh api graphql -f query="mutation { addSubIssue(input: {issueId: \"$EPIC_ID\", subIssueId: \"$STORY_ID\"}) { issue { id } subIssue { id } } }"
```

### Issue Status Labels

Issues move through this state machine:

```
status/backlog → status/ready → status/in-progress → status/review → status/done
```

- `status/backlog`: Blocked — has unresolved `depends-on`
- `status/ready`: All dependencies closed, available for pickup
- `status/in-progress`: Agent is working on it
- `status/review`: PR is open
- `status/done`: Merged and closed

### AGENT-META Blocks

Every story issue MUST include structured metadata:

```html
<!-- AGENT-META
parent: #13
depends-on: #14, #15
module: time
cpp-reference: upstream/ql/time/daycounters/actualactual.hpp
python-target: quantlib/time/daycounters/actualactual.py
-->
```

The `auto-advance-deps.yml` workflow parses `depends-on` to automatically unblock issues.

---

## Implementation: TDD via C++ Test Transpilation

Do NOT write tests from scratch. The C++ QuantLib test suite (`upstream/test-suite/`) is the source of truth.

1. **TRANSPILE** — Read the C++ Boost.Test file, generate equivalent pytest in `tests/`
2. **RED** — Run tests, confirm they fail (module doesn't exist yet)
3. **GREEN** — Write minimum Python code in `quantlib/` to make tests pass
4. **REFACTOR** — Clean up if needed

### Tracer Bullet Development

Work narrow and deep. Complete one area end-to-end before starting the next. Never go wide.

### API Compatibility

All classes use **camelCase** method names to match the C++ QuantLib API. This is deliberate — it's a drop-in replacement. Ruff naming rules N801/N802/N803/N806 are suppressed in `pyproject.toml`.

### Module Context

Before touching any module, read its context file at `.claude/memory/modules/<module>.md`. This contains architecture, classes, APIs, test coverage, and gaps vs C++. Update it after completing work.

---

## Verification: CI/CD Pipeline

### CI (`.github/workflows/python-tests.yml`)

Runs on **every PR** (no path filters):

```
make install → make lint → make typecheck → make test-cov
```

- **Lint**: `ruff check` + `ruff format --check` (pinned to v0.15.x)
- **Typecheck**: `pyright` (basic mode)
- **Test**: `pytest` with coverage (75% floor via `pyproject.toml`)
- **Matrix**: Python 3.10, 3.11, 3.12

### Pre-commit Hooks

Ruff check + format on every commit. Version pinned to match CI.

### Branch Protection (master = production)

| Setting | Value |
|---|---|
| Required status checks | `test (3.10)`, `test (3.11)`, `test (3.12)` |
| `strict: true` | Branch must be up-to-date with master before merging |
| Squash merge only | Merge commit and rebase disabled |
| Required linear history | Enabled |
| Force pushes | Disabled |
| Auto-merge | Enabled (squash) |
| Delete branch on merge | Enabled |

### Parallel Agent Safety

With `strict: true`, if Agent A merges first, Agent B's PR becomes stale. GitHub auto-updates the branch, CI re-runs against new master. If it passes, it merges. If it conflicts, it's blocked. This serializes parallel merges safely.

---

## Delivery: Automation Workflows

### `auto-merge.yml`
Enables GitHub auto-merge (squash) on every PR. When CI passes, the PR merges automatically.

### `auto-advance-deps.yml`
When an issue closes, scans `status/backlog` issues for `depends-on` references. If all dependencies are now closed, moves the issue to `status/ready`.

### `issue-lifecycle.yml`
- When a PR opens with "Closes #N": moves the linked issue to `status/review`
- When a sub-issue closes: checks if the parent epic's sub-issues are all done. If so, auto-closes the epic.

### `check-subissues.yml`
Prevents closing epics that have open sub-issues. Reopens the epic and posts a comment listing the remaining open sub-issues.

---

## Ralph Loop (Sustained Autonomous Work)

For continuous implementation across multiple issues, use the Ralph Loop:

```
/ralph-loop "Pick up the next status/ready GitHub Issue from itayfeldman/QuantLib and implement it using TDD..." --completion-promise "TASK COMPLETE" --max-iterations 15
```

Each iteration: pick next ready issue → implement via TDD → push → create PR → loop repeats.

---

## Model Selection

Use the cheapest model that can reliably complete each task. See `.claude/model-selection.md`.

| Task | Model |
|---|---|
| GitHub/git ops, labels, CI monitoring, simple enums | haiku |
| Test transpilation, pattern-following implementations | sonnet |
| Complex algorithms, architecture, debugging | opus |

Start cheap, escalate on failure.

---

## Dispatch Work Skill

Use `/dispatch-work` to pick up the next ready issue. The skill:

1. Queries `status/ready` issues
2. Verifies dependencies are closed
3. Classifies complexity → selects model tier
4. Reads module context
5. Creates worktree + branch
6. Executes TDD cycle
7. Pushes, creates PR with `Closes #N`
8. Updates module context if architecture changed

---

## Repository Structure

```
├── quantlib/                    # Python source (the drop-in replacement)
│   └── time/                    # ql/time module (Date, Period, Calendar, etc.)
│       ├── daycounters/         # Day count conventions
│       └── calendars/           # Market calendars
├── tests/                       # pytest test suite (mirrors quantlib/ structure)
├── upstream/                    # Original C++ QuantLib (READ-ONLY reference)
│   ├── ql/                      # C++ headers/source to translate from
│   └── test-suite/              # C++ Boost.Test files to transpile
├── .claude/
│   ├── lessons.md               # SDLC system design document
│   ├── model-selection.md       # Per-task model tier guide
│   ├── memory/modules/          # Per-module context snapshots
│   └── skills/dispatch-work/    # Agent skill for picking up work
├── .github/workflows/
│   ├── python-tests.yml         # CI pipeline (lint + typecheck + test + coverage)
│   ├── auto-merge.yml           # Enable auto-merge on PRs
│   ├── auto-advance-deps.yml    # Unblock dependent issues
│   ├── issue-lifecycle.yml      # Status transitions + epic auto-close
│   └── check-subissues.yml      # Prevent premature epic closure
├── CLAUDE.md                    # This file — loaded every session
├── pyproject.toml               # Python config (deps, ruff, pyright, coverage)
├── .pre-commit-config.yaml      # Pre-commit hooks (ruff v0.15.6)
└── Makefile                     # make test, make lint, make typecheck, make test-cov
```

---

## Make Targets

| Target | What |
|---|---|
| `make install` | Install package + dev deps + pre-commit hooks |
| `make test` | Run pytest |
| `make test-cov` | Run pytest with coverage (75% floor) |
| `make lint` | ruff check + ruff format --check |
| `make typecheck` | pyright quantlib/ |
| `make format` | Auto-format with ruff |
| `make ci` | Run lint + typecheck + test locally |
