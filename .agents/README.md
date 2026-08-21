# `.agents/` — On-Demand Guides for AI Agents

`AGENTS.md` at the repository root is loaded into an agent's context at the
start of *every* session, so it is deliberately kept short: hard constraints,
coding conventions, build/test entry points, pitfalls, and pointers.

The files in this directory hold the material that is only needed for a
*specific* task — adding an instrument, registering files in the build systems,
deprecating an API. `AGENTS.md` links to them; an agent reads one only when the
task calls for it, so they cost no context otherwise.

## Contents

| File | Read it when |
| --- | --- |
| [`architecture.md`](architecture.md) | You need the instrument/engine lifecycle, the recalculation triggers, the cash-flow hierarchy, or the design patterns. |
| [`build-and-test.md`](build-and-test.md) | You are configuring a build, picking build options, running the test suite, or checking which CI workflow covers a change. |
| [`registering-new-files.md`](registering-new-files.md) | You add or remove a `.hpp`/`.cpp` file and must update CMake, Autotools, and the Visual Studio projects. |
| [`extending-quantlib.md`](extending-quantlib.md) | You are adding a new instrument, term structure, calendar, or day counter. |
| [`deprecation.md`](deprecation.md) | You are deprecating or removing a public API. |
| [`maintaining-agent-docs.md`](maintaining-agent-docs.md) | You are editing `AGENTS.md` or one of the files above. |

## Why this directory

There is no cross-agent standard location for supplementary instruction files.
The [`AGENTS.md` convention](https://agents.md/) specifies only the file itself
(plus nested `AGENTS.md` files, which are path-scoped rather than task-scoped).
Task-scoped, load-on-demand instructions *are* standardized by the
[Agent Skills specification](https://agentskills.io/specification), but the
directories agents discover skills in remain vendor-specific:
`.claude/skills/` for [Claude Code][cc-skills], `.agents/skills/` and
`.opencode/skills/` for [opencode][oc-skills].

Plain Markdown referenced by relative path from `AGENTS.md` is the option that
works with every agent that can read a file, and with humans, without depending
on any one vendor's discovery rules. `.agents/` borrows its name from the
vendor-neutral skills path. If these guides later warrant automatic activation,
they can be promoted to `SKILL.md` files without changing anything here.

What belongs in these files, and what belongs in `AGENTS.md` instead, is
[`maintaining-agent-docs.md`](maintaining-agent-docs.md).

[cc-skills]: https://code.claude.com/docs/en/skills
[oc-skills]: https://opencode.ai/docs/skills/
