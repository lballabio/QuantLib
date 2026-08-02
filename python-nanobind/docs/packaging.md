# Packaging qlnb

## Why wheels must be built from a full checkout

`qlnb` statically links QuantLib compiled from the **parent** source tree
(`python-nanobind/../`). The scikit-build CMake project sets
`QLNB_BUILD_QUANTLIB=ON` and `add_subdirectory(..)`.

Implications:

- A PyPI-style **sdist of `python-nanobind/` alone cannot build** — the parent
  QuantLib headers/sources are missing.
- Build wheels (and local editable installs) from a full QuantLib git checkout:

  ```bash
  git clone <this-repo>
  cd QuantLib/python-nanobind
  pip install --no-build-isolation .
  ```

- CI uses [cibuildwheel](https://cibuildwheel.pypa.io/) with
  `package-dir: python-nanobind` while checking out the whole repository so the
  parent tree is visible to CMake.

## Stable ABI

`pyproject.toml` sets `wheel.py-api = "cp312"`. The extension is built with
nanobind's Stable ABI (`STABLE_ABI`), producing an abi3 wheel usable on
CPython 3.12+.

## Local wheel / sdist notes

```bash
# From python-nanobind/ — wheel works because parent tree exists
pip install build
python -m build --wheel

# sdist contains only the package tree metadata/sources; do not expect
# `pip install dist/qlnb-*.tar.gz` to succeed outside this monorepo layout.
python -m build --sdist
```

## cibuildwheel

Configuration lives in `[tool.cibuildwheel]` inside `pyproject.toml`. The
GitHub Actions workflow `.github/workflows/qlnb-wheels.yml` builds
`cp312-manylinux_x86_64` by default (full static QuantLib makes multi-OS
matrices expensive). Artifacts are uploaded from CI; publishing to PyPI is out
of scope for phase 3.
