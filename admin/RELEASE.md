# RELEASE PROCESS

## Before packaging

### Complete the set of features we want to include

- Any PR or issues we still want to include in this release?
- Anything newly added in this release that we still want to export
  through SWIG?

### Check wrappers for backward compatibility

In particular, run a diff between Python wrappers for the current and
the previous release and check that we're not losing kwargs for some
calls (because of overloading, or because SWIG added them
automatically for simple types but needs to be told for more complex
ones).  Generate the .py wrappers, download the old ones, run the
diff.  If possible, restore the lost kwargs.  If not, write them down
and add the info to the release notes.

### Clean up code

- Run the "Apply clang-tidy fixes" workflow if something changed since
  its last weekly run.
- Check the output of the test suite with `--log_level=message`.  Any
  missing trailing `...`? Any new messages that need editing? To avoid
  checking old messages, run the test suite from the previous release
  and look at the diffs.

### Check for unexpected changes

- Run the examples, compare the output with the previous release
- Same for the benchmark

### Clean up documentation

This goes for both QuantLib and QuantLib-SWIG.

Get the list of PRs merged in this release.  The easiest way is to go
to the "Pull requests" page on GitHub and filter on
`is:pr is:merged base:master merged:>=2026-04-14`
using the date of the last release.  Then:

- look at the PR titles and clean them up if needed;
- update the `Contributors.txt` file; find new contributors by
  filtering the list of merged PRs with `commenter:boring-cyborg[bot]`;
- update the `News.md` file; no need to list the PRs (the release
  workflow will do that) but we should:
  - point at the PR that removed deprecated code;
  - list the newly deprecated features (look for "Deprecated in version 1.NN" in the code);
  - list changes ahead, if any (for instance, if we're about to drop
    support for an old compiler or standard)
  See `https://github.com/lballabio/QuantLib/blob/v1.42/News.md` for an example.
  Don't use hard EOL, otherwise they end up as awkward breaks on the release page.

If needed (i.e., if we added or removed configuration options), also
update `Docs/pages/config.docs`.


## Create release candidate

### Update version number

Run the "Prepare release candidate" workflow.  It will prepare a PR
that increases the version number.  Let the PR run its checks and
merge (rebase) it.

Do the same for QuantLib-SWIG, but before running the "Prepare release
candidate" workflow in the SWIG repo make sure to regenerate the
devenv Docker images with the "Build quantlib-swig-devenv Docker
images" workflow so that the checks will run against the C++ release
candidate.

### Run checks

For the C++ library, the convenience workflow "Pre-release checks"
runs a number of workflows that check the library with a variety of
compilers and configurations.

For the wrappers, the workflows "Build C# nupkg" and "Build Python
wheels" check that we can build binaries correctly.  After the build
and checks, cancel the run: we must not publish the resulting
packages.

In case of errors, fix them and repeat.

### Publish prerelease

Pull the latest master locally and run

    git tag <tag>

where `tag` is, for instance, `v1.43-rc`.  No need to sign it, we'll
remove it when we create the final release.  Push the tag to GitHub.
This will trigger the "Publish release candidate" workflow.

Do the same for QuantLib-SWIG.  In this case, pushing the tag will
trigger two workflows that will build the .Net nupkg and the Python
wheels.  In this case, publish to TestPyPI and NuGetTest but
don't publish to PyPI and NuGet.

Finally, post to the quantlib-users mailing list, announce the release
candidate and ask for feedback.

### Wait for feedback

Leave the prerelease out for a couple of weekends so people can report
any problems. Fix those.


## Create release

### Update version number

Run the "Prepare release" workflow.  It will prepare a PR that
increases the version number.  Let the PR run its checks and merge
(rebase) it.

Do the same for QuantLib-SWIG, but before running the "Prepare
release" workflow in the SWIG repo make sure to regenerate the devenv
Docker images with the "Build quantlib-swig-devenv Docker images"
workflow so that the checks will run against the C++ release
candidate.

### Run checks

For the C++ library, the convenience workflow "Pre-release checks"
runs a number of workflows that check the library with a variety of
compilers and configurations.

For the wrappers, the workflows "Build C# nupkg" and "Build Python
wheels" check that we can build binaries correctly.  After the build
and checks, cancel the run: we must not publish the resulting
packages.

In case of errors, fix them and repeat.

### Tag the release

Pull the latest master locally and run

    git tag -s -F News.md <tag>

where `tag` is, for instance, `v1.43`.  Push the tag to GitHub.  This
will trigger the "Draft new release" workflow.  It will create
tarballs, remove the prerelease, publish a draft release, run Doxygen
and create a PR with the updated documentation on the repository for
the web site.

Do the same for QuantLib-SWIG.  In this case, the "Draft new release"
workflow will create the tarballs and the draft release and remove the
prerelease.  Pushing the tag will also trigger the workflows that will
build the .Net nupkg and the Python wheels.  Publish to TestPyPI and
NuGetTest and wait for the rest.

### Publish the release

At this point, you should only need to push a few buttons:

- merge (rebase) the PR for the new documentation on the web site;
- publish the draft releases (C++ and SWIG) on GitHub (you need to click
on the "edit" pencil icon);
- deploy the nupkg on NuGet;
- deploy the wheels on PyPI.

Publishing the release will also trigger a few workflows.  Two (one on
QuantLib and one on QuantLib-SWIG) will create PRs that increase the
version number.  Another one will move the "binder" branch on
QuantLib-SWIG to the release tag.

### Announce the release

Send an email about the release to the QuantLib users, dev and
announce mailing lists; use an old one as template and change the
version number and links.  Optionally announce on other social sites.

## After the release

### Versioning

Merge the PRs created by the "Increase version number" workflow on
both repos.

### Deprecated features

Open a PR to remove the features deprecated in past releases.  At this
time we're using a 5-releases lag; version 1.44 removes the features
deprecated in version 1.39 and so on.
