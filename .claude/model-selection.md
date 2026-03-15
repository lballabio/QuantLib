# Model Selection Guide

Use the cheapest model that can reliably complete each task. When spawning subagents via the Agent tool, set the `model` parameter accordingly.

## Model Tiers

### haiku (cheapest, fastest)
Use for tasks that are mechanical, well-defined, or follow a clear template.

**Tasks:**
- GitHub operations: label changes, issue status updates, closing/opening issues
- Git operations: branch creation, pushing, fetching
- CI monitoring: watching runs, checking status
- Creating GitHub issues/PRs from a known template
- Simple enum implementations (direct value mapping from C++ header)
- Wiring sub-issues (GraphQL mutations)
- File creation with known content (\_\_init\_\_.py, simple configs)
- Running make targets and reporting results

### sonnet (balanced)
Use for tasks that require understanding code structure but follow established patterns.

**Tasks:**
- Test transpilation from C++ to pytest (when the C++ test is straightforward)
- Implementing classes that follow an established pattern (e.g., new DayCounter subclass matching existing ones like Actual360)
- New calendar implementations (holiday rules are mechanical but need careful reading)
- Writing MakeSchedule-style builders
- Fixing lint/typecheck errors
- Extending existing test files with new test cases
- PR creation with meaningful summaries
- Module context file updates

### opus (most capable, most expensive)
Use for tasks that require deep reasoning, architecture decisions, or complex logic.

**Tasks:**
- Complex algorithm implementation (ActualActual ISMA, Schedule generation with stubs)
- Designing new module architecture (choosing patterns, API surface)
- Transpiling complex C++ tests with intricate setup/assertions
- Debugging test failures that aren't obvious
- Implementing comparison/ordering logic with edge cases
- Creating epics and breaking down large features into stories
- Code review of complex implementations
- Any task where a cheaper model has already failed

## How to Apply

### In dispatch-work skill
After reading the issue, classify it before starting work:

```
# Read the issue body and classify
if issue has labels like "simple", or body describes enum/config:
    → haiku tasks where possible, sonnet for implementation
elif issue follows established patterns (new day counter, new calendar):
    → sonnet for the whole thing
elif issue involves new patterns, complex algorithms, architecture:
    → opus
```

### When spawning parallel agents
```python
# GitHub ops → haiku
Agent(model="haiku", prompt="Update issue #N labels...")

# Pattern-following implementation → sonnet
Agent(model="sonnet", prompt="Implement Actual364 day counter following the Actual360 pattern...")

# Complex logic → opus
Agent(model="opus", prompt="Implement ActualActual ISMA with schedule-based year fraction...")
```

### Cost-saving heuristics
1. **Start cheap, escalate on failure.** Try sonnet first. If it produces bugs or wrong logic, retry with opus.
2. **Split tasks by complexity.** A single issue might need opus for the algorithm but haiku for the git/GitHub ops around it.
3. **Use haiku for verification.** Running tests, checking CI, reading results — these don't need intelligence.
4. **Read module context first.** The `.claude/memory/modules/` files eliminate expensive re-analysis. Always read before starting work.
