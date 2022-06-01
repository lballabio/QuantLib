Changes for QuantLib 1.26:
==========================

QuantLib 1.26 includes 26 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/22?closed=1>.

Portability
-----------

- **End of support:** as announced in the notes for the previous
  release, this release is the last to support Visual Studio 2013.

- **End of support:** this release is the last to support the
  long-deprecated configure switches `--enable-disposable` and
  `--enable-std-unique-ptr`.  From the next release, `Disposable` will
  always be disabled (and eventually removed) and `std::unique_ptr`
  will always be used instead of `std::auto_ptr`.  This has already
  been the default in the last few releases.

- **Future end of support:** this release and the next will be the
  last to avoid C++14 syntax.  This should still support most
  compilers released in the past several years (except for Visual
  Studio 2013, which we're already dropping in this release).

- If tagged libraries are specified, as is the default on Windows,
  CMake now gives the built libraries the same names as the Visual
  Studio solution (for instance, `QuantLib-x64-mt-s` instead of
  `QuantLib-mt-s-x64`) so that the pragma in `ql/auto_link.hpp` works.

- QuantLib can now also be built as a subproject in a larger CMake
  build (thanks to Peter Caspers).

Date/time
---------

- When printed, `Period` instances now display transparently what
  their units and length are, instead of doing more fancy formatting
  (e.g., "16 months" is now displayed instead of "1 year 4 months").
  Also, `Period` instances that compare as equal now return the same
  period from their `normalize` method.

Indexes
-------

- Added Tona (Tokyo overnight average) index (thanks to Jonghee Lee).

- Added static `laggedFixing` method to `CPI` structure which provides
  interpolation of inflation index fixings.

Cash flows
----------

- The `CPICoupon` and `CPICashFlow` classes now take into account the
  correct dates and observation lag for interpolation.

Instruments
-----------

- Added a `BondForward` class that generalizes the existing
  `FixedRateBondForward` to any kind of bond (thanks to Marcin
  Rybacki).

- Avoided unexpected jumps in callable bond OAS (thanks to Ralf Konrad).

- Fixed `TreeSwaptionEngine` mispricing when adjusting the instrument
  schedule to a near exercise date (thanks to Ralf Konrad).

- the `ForwardRateAgreement` class now works correctly without an
  explicit discount curve.

Term structures
---------------

- Dates explicitly passed to `InterpolatedZeroInflationCurve` are no
  longer adjusted automatically to the beginning of their inflation period.

Deprecated features
-------------------

- **Removed** the `MCDiscreteAveragingAsianEngine` class, deprecated
  in version 1.21.

- Deprecated the `LsmBasisSystem::PolynomType` typedef, now renamed to
  `PolynomialType`; `MakeMCAmericanEngine::withPolynomOrder` was also
  deprecated and renamed to `withPolynomialOrder`.

- Deprecated the `ZeroInflationCashFlow` constructor taking an unused
  calendar and business-day convention.

- Deprecated the `CPICoupon` constructor taking a number of fixing
  days, as well as the `CPICoupon::indexObservation`,
  `CPICoupon::adjustedFixing` and `CPICoupon::indexFixing` methods
  and the `CPILeg::withFixingDays` method.

- Deprecated the `CPICashFlow` constructor taking a precalculated fixing date and a frequency.

- Deprecated the `Observer::set_type` and `Observable::set_type` typedefs.

- Deprecated the unused `Curve` class.

- Deprecated the unused `LexicographicalView` class.

- Deprecated the unused `Composite` class.

- Deprecated the unused `DriftTermStructure` class.


**Thanks go also** to Matthias Groncki, Jonathan Sweemer and Li Zhong
for smaller fixes, enhancements and bug reports.
