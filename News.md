Changes for QuantLib 1.43
=========================

Features deprecated in release 1.38 were removed in this release; see <https://github.com/lballabio/QuantLib/pull/2539> for a full list.

A number of features were deprecated in this release and will be removed in a future release (probably release 1.48):

- The `AsIndex` element of the `CPI::InterpolationType` enumeration; use either `Linear` or `Flat`.
- The `YoYInflationIndex::interpolated` method and the corresponding `interpolated_` data member; indexes no longer interpolate, coupons do.
- The `Settlement` element of the `Israel::Market` enumeration; use an explicit market (TASE, Shir or Telbor).
- Test selection based on speed (that is, passing `--slow`, `--fast` or `--faster` to the test suite) is no longer supported.


Full list of pull requests
--------------------------

All the pull requests merged in this release are listed on its release page at <https://github.com/lballabio/QuantLib/releases/tag/v1.43>.

The list of commits since the previous release is available in `ChangeLog.txt`.

