# Maintaining the Agent Guides

Companion to [`AGENTS.md`](../AGENTS.md). Read this when you are editing
`AGENTS.md` itself or a file in this directory — not when you are working on
QuantLib.

## 1. Instructions, Not Overviews

Write what an agent should *do* or *avoid*, not what the repository *contains*.
Directory trees, file inventories, and "what lives where" tours do not
measurably help an agent find the right file, they go stale, and `ls` and `grep`
answer the same question on demand. Prefer a rule an agent cannot derive from
the codebase in one command.

The evidence is [Gloaguen et al., *Evaluating AGENTS.md*][paper]. Across
SWE-bench and a purpose-built benchmark of repositories carrying
developer-written context files, agents reliably *followed* instructions given
in those files, but repository overviews did not improve their results — while
the files raised inference cost by over 20%.

Two caveats before treating that as settled. The overview finding is "does not
earn its tokens" rather than "actively harmful": removing the overview section
changed accuracy insignificantly in the authors' own ablation. And the study is
Python-only, which the authors flag first under Limitations — for a
well-represented language, tooling knowledge may already sit in the model's
weights. QuantLib's three parallel build systems, hand-maintained `.vcxproj`
files, `tools/check_filelists.sh`, and N+5 deprecation cadence are the opposite
case, and are worth writing down.

## 2. Smallest Scope That Works

`AGENTS.md` is loaded at the start of *every* session, whatever the task, so a
line there is paid for by every agent forever. A line here is paid for only by
the agent that needed it.

- Needed on *every* task → `AGENTS.md`.
- Needed for *one kind* of task → a file in this directory, plus a row in the
  `AGENTS.md` task-guide table so an agent knows when to load it.

Corollary: keep external links out of `AGENTS.md`. An agent that finds a URL in
its always-loaded context may fetch it before doing the work it was asked to do.
Links belong in these on-demand files, where the reader has already chosen the
topic.

## 3. Accuracy and Cadence

Verify a claim against the repository before writing it down. A guide that
confidently states something false is worse than no guide, because an agent has
no reason to doubt it.

Update `AGENTS.md` when build/test entry points, workflow files, or major
conventions change:

- quick check before release candidates
- deeper full pass quarterly

[paper]: https://arxiv.org/abs/2602.11988
