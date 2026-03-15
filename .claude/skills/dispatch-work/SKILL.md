---
name: dispatch-work
description: Pick up the next ready GitHub Issue and start working on it. This skill should be used when the user says "dispatch", "next task", "pick up work", or wants to start implementing the next available issue.
---

# Dispatch Work

Pick up the next ready GitHub Issue and execute it using TDD with cost-efficient model selection.

## Workflow

### 1. Find the next issue
```
gh issue list --repo itayfeldman/QuantLib --label "status/ready" --json number,title,labels,body
```

### 2. Verify dependencies
Read the issue body. Check `<!-- AGENT-META -->` blocks for `depends-on`. Verify all deps are closed. Skip if not.

### 3. Classify complexity and select model tier
Read `.claude/model-selection.md` for the full guide. Quick classification:

| Signal in issue | Tier | Model |
|---|---|---|
| "enum", "config", single file, value mapping | Simple | haiku for git ops, sonnet for impl |
| "follows X pattern", new day counter/calendar, extends existing | Standard | sonnet |
| "new class", "algorithm", complex C++ logic, architecture | Complex | opus |

### 4. Update issue status
```
gh issue edit <NUMBER> --repo itayfeldman/QuantLib --remove-label "status/ready" --add-label "status/in-progress"
```

### 5. Read module context
Before touching any code, read the relevant module context file:
```
.claude/memory/modules/<module>.md
```
This gives you the full architecture, existing classes, patterns, and gaps — no need to re-analyze.

### 6. Create a branch
```
git fetch origin
git checkout -b task/<NUMBER>-<short-description> origin/master
```

### 7. Execute with model-appropriate agents
Split work into parallel agents where possible:

**For simple issues** — dispatch sonnet agent for implementation:
```
Agent(model="sonnet", prompt="Implement <X>. Read .claude/memory/modules/<mod>.md for context. Follow the <ExistingClass> pattern. TDD: write tests first, then implement.")
```

**For complex issues** — use opus for the core logic, haiku for surrounding ops:
```
Agent(model="opus", prompt="Implement <complex algorithm>...")
Agent(model="haiku", prompt="Run make lint && make typecheck && make test and report results")
```

### 8. Verify
```
make lint && make typecheck && make test-cov
```

### 9. Commit, push, and open PR
```
git add <files>
git commit -m "<descriptive message>"
git push -u origin task/<NUMBER>-<short-description>
gh pr create --repo itayfeldman/QuantLib --title "<title>" --body "Closes #<NUMBER>..."
```
PR auto-merges when CI passes.

### 10. Update module context
If the implementation changed the module's architecture, classes, or coverage, update `.claude/memory/modules/<module>.md`.

## Rules

- ALWAYS use `--repo itayfeldman/QuantLib` with `gh` commands. NEVER target `lballabio/QuantLib`.
- One issue per dispatch. Don't batch multiple issues.
- If no issues are `status/ready`, report that to the user.
- If tests can't pass due to missing dependencies, stop and explain rather than hacking around it.
- Start with the cheapest viable model. Escalate to opus only if sonnet fails.
- Always read module context before implementation to save tokens.
