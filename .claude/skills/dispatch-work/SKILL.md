---
name: dispatch-work
description: Pick up the next ready GitHub Issue and start working on it. This skill should be used when the user says "dispatch", "next task", "pick up work", or wants to start implementing the next available issue.
---

# Dispatch Work

Pick up the next ready GitHub Issue and execute it using TDD.

## Workflow

1. **Find the next issue** — Query GitHub Issues for `itayfeldman/QuantLib` with label `status/ready`, sorted by issue number (lowest first):
   ```
   gh issue list --repo itayfeldman/QuantLib --label "status/ready" --sort created --json number,title,labels,body
   ```

2. **Verify dependencies** — Read the issue body. Check for `<!-- AGENT-META -->` blocks listing `depends-on` issues. Verify all dependencies are closed. If not, skip to the next ready issue.

3. **Update issue status** — Move the issue to in-progress:
   ```
   gh issue edit <NUMBER> --repo itayfeldman/QuantLib --remove-label "status/ready" --add-label "status/in-progress"
   ```

4. **Create a branch** — Branch from `master`:
   ```
   git checkout master && git pull
   git checkout -b task/<NUMBER>-<short-description>
   ```

5. **Execute the issue** — Follow the technical approach in the issue body. Use TDD:
   - **TRANSPILE** — Read the C++ test file from `upstream/test-suite/`, generate pytest equivalent in `tests/`
   - **RED** — Run `make test`, confirm tests fail
   - **GREEN** — Write minimum Python code in `quantlib/` to make tests pass
   - **REFACTOR** — Clean up if needed

6. **Verify** — Run the full check suite:
   ```
   make lint && make typecheck && make test
   ```

7. **Commit, push, and open PR**:
   ```
   git add <files>
   git commit -m "<descriptive message>"
   git push -u origin task/<NUMBER>-<short-description>
   gh pr create --repo itayfeldman/QuantLib --title "<title>" --body "Closes #<NUMBER>\n\n## Summary\n..."
   ```

8. **Update issue status**:
   ```
   gh issue edit <NUMBER> --repo itayfeldman/QuantLib --remove-label "status/in-progress" --add-label "status/in-review"
   ```

## Rules

- ALWAYS use `--repo itayfeldman/QuantLib` with `gh` commands. NEVER target `lballabio/QuantLib`.
- One issue per dispatch. Don't batch multiple issues.
- If no issues are `status/ready`, report that to the user.
- If tests can't pass due to missing dependencies, stop and explain rather than hacking around it.
