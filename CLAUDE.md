@AGENTS.md

## Fork-specific notes

This fork adds FX smile-section, local-vol and stochastic-local-vol
(SLV) functionality. All of it lives in `ql/experimental/fxslv/`, with
tests in `test-suite/fxlocalvol.cpp`, `fxslvmodel.cpp` and
`fxsmilesections.cpp`, and examples in `Examples/FXLocalVol` and
`Examples/FXSmileSections`.

When adding a file to the fork, keep it in `ql/experimental/fxslv/` and
register it in `ql/CMakeLists.txt`, `ql/experimental/fxslv/Makefile.am`,
`ql/experimental/fxslv/all.hpp`, `QuantLib.vcxproj` and
`QuantLib.vcxproj.filters`. Avoid editing upstream's file lists
elsewhere, so that merges from lballabio/QuantLib stay conflict-free.
