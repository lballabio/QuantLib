# Agentic SDLC: How This Project Runs

This document describes the software development lifecycle implemented in this repo, where Claude agents do the planning, coding, testing, and delivery — with GitHub as the orchestration layer.

---

## Overview

The SDLC is a loop: **Plan → Implement → Verify → Deliver → Repeat**. GitHub Issues define the work. GitHub Actions automate the pipeline. Claude agents execute the tasks. Humans steer by creating epics and reviewing results.

```
Human: "Build day count conventions"
  │
  ▼
┌─────────────────────────────────────────────┐
│  PLAN (Claude + GitHub Issues)              │
│  Create Epic → break into Stories →         │
│  wire sub-issues → set status labels        │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  IMPLEMENT (Claude Agent + Ralph Loop)      │
│  Pick status/ready issue → branch →         │
│  transpile C++ tests → TDD red/green →      │
│  commit → push → create PR                  │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
┌─────────────────────────────────────────────┐
│  VERIFY + DELIVER (GitHub Actions)          │
│  CI (lint/typecheck/test+coverage) →        │
│  auto-merge → close issue →                 │
│  advance deps → auto-close epic             │
└──────────────────┬──────────────────────────┘
                   │
                   ▼
              Next issue
```

---

## Planning Layer: GitHub Issues

All planning lives in GitHub Issues. No local files, no task lists, no wikis for planning.

### Issue Hierarchy

- **Epics** (`type/epic`): Define "what" we're building. Acceptance criteria, scope, C++ references. Example: "Epic: Day Count Conventions"
- **Stories** (`type/story`): Define "how" to build it. Technical approach, files to create, C++ files to reference. Example: "Story: Actual/Actual day counters (ISDA, ISMA, AFB)"

Epics and stories are linked via **GitHub sub-issues** (not markdown links). This uses the GraphQL `addSubIssue` mutation:

```bash
# Get node IDs
EPIC_ID=$(gh api graphql -f query='query { repository(owner: "itayfeldman", name: "QuantLib") { issue(number: 20) { id } } }' --jq '.data.repository.issue.id')
STORY_ID=$(gh api graphql -f query='query { repository(owner: "itayfeldman", name: "QuantLib") { issue(number: 21) { id } } }' --jq '.data.repository.issue.id')

# Wire parent-child
gh api graphql -f query="mutation { addSubIssue(input: {issueId: \"$EPIC_ID\", subIssueId: \"$STORY_ID\"}) { issue { id } subIssue { id } } }"
```

### Issue Status Labels

Issues move through a state machine via labels:

```
status/backlog → status/ready → status/in-progress → status/review → status/done
```

- `status/backlog`: Blocked by dependencies (has unresolved `depends-on`)
- `status/ready`: All dependencies closed, available for pickup
- `status/in-progress`: Agent is working on it
- `status/review`: PR is open, CI running
- `status/done`: Merged and closed

### AGENT-META Blocks

Each story issue contains structured metadata in an HTML comment:

```html
<!-- AGENT-META
parent: #13
depends-on: #14
module: time
cpp-reference: upstream/ql/time/daycounters/actualactual.hpp
python-target: quantlib/time/daycounters/actualactual.py
-->
```

This is machine-readable. The `auto-advance-deps.yml` workflow parses `depends-on` to automatically unblock issues when their dependencies close.

---

## Implementation Layer: Claude Agent

### The dispatch-work Skill

The `/dispatch-work` skill (`.claude/skills/dispatch-work/SKILL.md`) defines the agent's workflow for picking up and executing issues:

1. Query `status/ready` issues
2. Verify dependencies are closed
3. Classify complexity → select model tier
4. Read module context (`.claude/memory/modules/<module>.md`)
5. Create branch from `origin/master`
6. Execute via TDD: transpile C++ tests → RED → GREEN → REFACTOR
7. Run `make lint && make typecheck && make test-cov`
8. Commit, push, create PR with `Closes #N`

### Ralph Loop

For sustained autonomous work, the Ralph Loop (`/ralph-loop`) feeds the same prompt to Claude repeatedly. Each iteration:

1. Claude receives the dispatch prompt
2. Picks up the next ready issue and implements it
3. Pushes code, creates PR
4. Loop re-triggers with the same prompt
5. Claude sees its previous work in files/git and picks up the next issue

The loop continues until all issues are done or `max-iterations` is reached.

### TDD via C++ Test Transpilation

Tests are NOT written from scratch. The C++ QuantLib test suite (`upstream/test-suite/`) is the source of truth:

1. **TRANSPILE**: Read the C++ Boost.Test file, generate equivalent pytest
2. **RED**: Run tests, confirm they fail (module doesn't exist yet)
3. **GREEN**: Write minimum Python code to make tests pass
4. **REFACTOR**: Clean up

This ensures the Python implementation matches the C++ library's behavior exactly.

### Model Selection

Different tasks use different Claude models for cost efficiency (`.claude/model-selection.md`):

| Task | Model | Why |
|---|---|---|
| GitHub/git ops, label changes, CI monitoring | haiku | Mechanical, no reasoning needed |
| Test transpilation, pattern-following implementations | sonnet | Understands code but follows templates |
| Complex algorithms, architecture, debugging | opus | Needs deep reasoning |

Rule: start with the cheapest viable model, escalate on failure.

### Module Context Memory

Per-module context files (`.claude/memory/modules/<module>.md`) are checked into the repo. They contain:

- Architecture and design patterns used
- Complete file listing with classes and APIs
- Test coverage summary
- Gaps vs C++ (what's not yet implemented)

Agents read these before starting work, eliminating the need to re-analyze the codebase each session. A 100-line context file replaces 2000+ tokens of file reading.

---

## Verification & Delivery Layer: GitHub Actions

### CI Pipeline (`python-tests.yml`)

Every PR runs on Python 3.10, 3.11, 3.12:

```
make install → make lint → make typecheck → make test-cov
```

- **Lint**: `ruff check` + `ruff format --check`
- **Typecheck**: `pyright`
- **Test**: `pytest` with coverage (75% floor via pyproject.toml)

### Pre-commit Hooks (`.pre-commit-config.yaml`)

Ruff check and format run on every local commit via `pre-commit`. Version is pinned to match CI (`v0.15.6`).

### Automated Workflows

| Workflow | Trigger | What it does |
|---|---|---|
| `auto-merge.yml` | PR opened | Enables GitHub auto-merge (squash) |
| `auto-advance-deps.yml` | Issue closed | Moves dependent `status/backlog` issues to `status/ready` |
| `issue-lifecycle.yml` | PR opened / Issue closed | Moves linked issues to `status/review`; auto-closes epics when all sub-issues done |
| `check-subissues.yml` | Issue closed | Reopens epics closed with open sub-issues |

### The Automated Merge Flow

```
Agent pushes code
  → Creates PR with "Closes #N"
    → CI runs (lint + typecheck + test + coverage)
      → auto-merge.yml enables squash auto-merge
        → CI passes → PR auto-merges
          → "Closes #N" closes the issue
            → auto-advance-deps.yml unblocks dependent issues
              → issue-lifecycle.yml checks if epic's sub-issues are all done
                → If yes: auto-closes epic
```

The agent's job ends at "creates PR". Everything after is event-driven.

### Branch Protection

Master branch requires all CI checks to pass. `enforce_admins` is off so auto-merge works. `delete_branch_on_merge` cleans up feature branches.

---

## Repo Structure

```
├── quantlib/                    # Python source (the drop-in replacement)
│   └── time/                    # ql/time module
│       ├── date.py, period.py   # Core classes
│       ├── daycounters/         # Day count conventions
│       └── calendars/           # Market calendars
├── tests/                       # pytest test suite
│   └── time/                    # Tests mirror source structure
├── upstream/                    # Original C++ QuantLib (read-only reference)
│   ├── ql/                      # C++ headers/source to translate from
│   └── test-suite/              # C++ tests to transpile
├── .claude/
│   ├── lessons.md               # This file
│   ├── model-selection.md       # Per-task model tier guide
│   ├── memory/modules/          # Per-module context snapshots
│   └── skills/dispatch-work/    # Agent skill for picking up work
├── .github/workflows/           # CI + automation workflows
├── CLAUDE.md                    # Project instructions for Claude
├── pyproject.toml               # Python project config
└── Makefile                     # Dev commands
```

---

## Parallel Agent Safety

When multiple Claude Code sessions run in parallel, each creating PRs against master, the following protections prevent broken code from merging:

### Branch Protection (Repo Settings)

| Setting | Value | Why |
|---|---|---|
| Required status checks | `test (3.10)`, `test (3.11)`, `test (3.12)` | Every PR must pass full CI |
| `strict: true` | Enabled | **Critical**: PR branch must be up-to-date with master before merging. If agent A merges first, agent B's PR becomes stale → GitHub auto-updates it → CI re-runs against new master |
| Required linear history | Enabled | No merge commits, clean squash-only history |
| Squash merge only | Enabled | Merge commit and rebase disabled |
| Force pushes | Disabled | No rewriting master history |
| Allow deletions | Disabled | Can't delete master |

### How `strict: true` Protects Parallel Agents

This is the key mechanism. Without a merge queue (requires GitHub Team plan), `strict: true` provides equivalent safety:

```
Agent A: branch → implement → push → PR passes CI → merges to master ✓
Agent B: branch → implement → push → PR passes CI → tries to merge
         → BLOCKED: branch is behind master (Agent A's code just merged)
         → GitHub auto-updates branch with new master
         → CI re-runs against master + Agent A's code + Agent B's code
         → If CI passes → merges ✓
         → If CI fails (conflict/regression) → merge blocked ✗
```

This means parallel agents can never merge conflicting code — the second agent's PR always gets re-tested against the first agent's merged changes.

### CI Coverage: No Path Filters

CI runs on **every PR regardless of which files changed**. No path filters. This prevents:
- Workflow changes (`.github/`) merging untested
- Config changes (`.claude/`, `CLAUDE.md`) skipping CI
- A PR that only adds docs but breaks an import

### Auto-Merge Behavior with Parallel PRs

The `auto-merge.yml` workflow enables squash auto-merge on every PR. With `strict: true`:

1. PR A passes CI → auto-merges immediately
2. PR B was also passing CI, but is now behind master
3. GitHub auto-updates PR B's branch (because `strict: true`)
4. CI re-runs on PR B against updated master
5. If CI passes → PR B auto-merges
6. If CI fails → PR B stays open, agent must fix

This creates a natural serialization: parallel agents race to merge, but the protection ensures each merge is tested against the latest master state.

### What's NOT Protected (Free Plan Limitations)

- **No merge queue**: GitHub merge queue (tests PRs in batches before merging) requires Team plan. `strict: true` is the free-plan equivalent.
- **No required reviewers**: PRs merge without human review. For this project that's intentional — agents are the primary contributors.
- **No CODEOWNERS**: No file-level ownership requiring specific reviewer approval.

---

## Configuration Files

| File | Purpose |
|---|---|
| `CLAUDE.md` | Top-level instructions loaded into every Claude session |
| `.claude/lessons.md` | This file — SDLC system design |
| `.claude/model-selection.md` | Maps task types to model tiers |
| `.claude/memory/modules/*.md` | Per-module architecture snapshots |
| `.claude/skills/dispatch-work/SKILL.md` | Agent workflow for issue execution |
| `pyproject.toml` | Python config: deps, ruff, pyright, coverage |
| `.pre-commit-config.yaml` | Pre-commit hooks (ruff, pinned version) |
| `Makefile` | Dev commands: test, lint, typecheck, test-cov |
