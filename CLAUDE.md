@AGENTS.md

## Fork-specific notes

This fork adds FX smile-section, local-vol and stochastic-local-vol
(SLV) functionality. All of it lives under `ql/experimental/fxslv/`:
market conventions (settlement, expiry cut, tenor dates) in its
`conventions/` subdirectory, smile sections in `smilesections/`, the
trading-time clock (its term structure, day counter and yield-curve
wrapper) in `tradingtime/`, with tests in
`test-suite/fxlocalvol.cpp`, `fxslvmodel.cpp` and `fxsmilesections.cpp`,
and examples in `Examples/FXLocalVol` and `Examples/FXSmileSections`.

The analytics take an `FxPairMarket` (one pair's conventions, spot,
curves, vol quotes and events, immutable, with the surface built from
them on first use) and an `FxPairConventions` (how a pair settles and
is quoted; the tenors and deltas quoted are market data, in
`FxVolQuotes`).  The vol surface's time axis is trading time: its
`blackVol` is per year of trading time and needs curves on the same axis,
`marketVol` gives quote-convention vols, and `FxCalendarTimeVolSurface`
the calendar axis for SLV until it runs in trading time.  They are meant
to be exposed through SWIG
to a Python layer, which owns the data loading, the conventions registry
and a market bundling several pairs, and builds the `FxPairMarket`s;
keep their public interfaces free of templates for that reason.

When adding a file to the fork, keep it under `ql/experimental/fxslv/`
and register it in `ql/CMakeLists.txt`, the `Makefile.am` and `all.hpp`
of its directory, `QuantLib.vcxproj` and `QuantLib.vcxproj.filters`
(under the filter of its directory). A new subdirectory also needs its
own `Makefile.am` and `all.hpp` (on the pattern of `smilesections/`), an
entry in the parent's `SUBDIRS` and `libFxSlv_la_LIBADD`, an include of
its `all.hpp` in the parent's, its `all.hpp` in `QuantLib.vcxproj*` and
in the generated headers of `ql/CMakeLists.txt` (`${PROJECT_BINARY_DIR}/...`),
a `<Filter>` in `QuantLib.vcxproj.filters` and its `Makefile` in
`configure.ac` next to `ql/experimental/fxslv/Makefile`;
`tools/check_filelists.sh` checks the lists.
That one line in `configure.ac` is the only upstream file list to edit;
avoid editing the others, so that merges from lballabio/QuantLib stay
conflict-free.
