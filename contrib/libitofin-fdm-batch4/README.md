# libitofin FDM Batch 4 — European Black-Scholes vertical

Working tree of a planned fork of [benbenbang/libitofin](https://github.com/benbenbang/libitofin)
implementing [issue #635](https://github.com/benbenbang/libitofin/issues/635) Batch 4.

This agent environment could not create `zhangys2/libitofin` (GitHub App 403 on
`createRepository` / `forks`), so the change is landed here as an apply-able
patch plus source copies for review. Local development clone:

```text
/home/ubuntu/git/libitofin   # branch feat/fdm-batch4-european-bs
QuantLib/ -> /workspace      # symlink oracle (this tree)
```

## Apply against upstream

```sh
git clone https://github.com/benbenbang/libitofin.git
cd libitofin
git am path/to/0001-feat-fdm-Batch-4-European-Black-Scholes-FD-vertical-635.patch
cargo test -p libitofin --lib methods::finitedifferences
cargo test -p libitofin --lib --release fdblackscholesvanillaengine
```

## What landed

| Piece | Path in libitofin |
|-------|-------------------|
| `FdmSolverDesc` | `crates/libitofin/src/methods/finitedifferences/solvers/fdmsolverdesc.rs` |
| `Fdm1DimSolver` | `.../solvers/fdm1dimsolver.rs` |
| `FdmBlackScholesSolver` | `.../solvers/fdmblackscholessolver.rs` |
| `FdBlackScholesVanillaEngine` | `crates/libitofin/src/pricingengines/vanilla/fdblackscholesvanillaengine.rs` |

Oracles: `testFdEngines` tolerances (value 1e-4, delta/gamma 1e-6, theta 1e-3)
at 500×500; Douglas + ImplicitEuler arms of `testPDESchemes`. Deferred to
[#636](https://github.com/benbenbang/libitofin/issues/636): local-vol, quanto,
dividends, American/Bermudan, SparseMatrix/Krylov, unported PDE schemes.

## Next step for a real upstream PR

Once a `zhangys2/libitofin` fork exists:

```sh
git remote add origin git@github.com:zhangys2/libitofin.git
git push -u origin feat/fdm-batch4-european-bs
gh pr create -R benbenbang/libitofin --fill
```
