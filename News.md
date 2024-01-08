Changes for QuantLib 1.33:
==========================

QuantLib 1.33 includes 38 pull requests from several contributors.

Some of the most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/31?closed=1>.


Portability
-----------

- **Future end of support:** as announced in release 1.32, we're
  targeting the future release 1.35 as the last to support Visual C++
  2015, g++ up to version 6.x, and clang up to version 4; support for
  those compilers will be dropped in release 1.36, about nine months
  from now.  From that point onwards, this will allows us to enable
  the use of C++17 in the code base.

- **Future end of support:** at the same time as the above, we'll also
  remove the configure switch that allows to use `boost::tuple`,
  `boost::function` and `boost::bind` instead of their `std`
  counterparts; the `std` classes are already the default since
  release 1.32.

- Added CMake presets for Apple; thanks to Christian Köhnenkamp
  (@kohnech).


Dates and calendars
-------------------

- Added New Year's Eve as a holiday to Chilean calendar; thanks to
  GitHub user @MoixaStrikes.

- Added Chinese holidays for 2024; thanks to Cheng Li (@wegamekinglc).

- Updated list of known ECB dates; thanks to GitHub user @PaulXiCao.

- Added Thailandese and Taiwanese holidays up to 2024; thanks to
  Fredrik Gerdin Börjesson (@gbfredrik).

- Added one-time holiday to South African calendar; thanks to Francois
  Botha (@igitur).


Models
------

- Added support for angled contour shift integrals to Heston model;
  thanks to Klaus Spanderen (@klausspanderen).


Instruments
-----------

- Allow different calendars and frequencies for different legs in
  `MakeOIS` and `OISRateHelper`; thanks to Eugene Toder (@eltoder).

- Enabled negative payment lag in swap legs; thanks to GitHub user
  @Stoozy.


Random numbers
--------------

- Added Burley 2020 scrambled Sobol sequence generator; thanks to
  Peter Caspers (@pcaspers).


Tests
-----

- Use automated registration of unit tests; thanks to Siddharth
  Mehrotra (@Sidsky).

- Added a few fuzzing tests; thanks to Nathaniel Brough (@silvergasp).

- Improved test coverage for a few classes; thanks to GitHub user
  @PaulXiCao.


Deprecated features
-------------------

- **Removed** features deprecated in version 1.28:
  - The overload of `CallableBond::impliedVolatility` taking an NPV as target.
  - The constructor of `AmortizingFixedRateBond` taking a sinking frequency.
  - The constructor of `AmortizingFixedRateBond` taking a vector of
    `InterestRate` instances.
  - The constructor of `FixedRateBond` taking start date, maturity
    date etc. instead of a schedule.
  - The constructor of `FixedRateBond` taking a vector of
    `InterestRate` instances.
  - The constructor of `FloatingRateBond` taking start date, maturity
    date etc. instead of a schedule.
  - The constructor of `CPICapFloor` taking a handle to an
    interest-rate index.
  - The `CPICapFloor::inflationIndex` method.
  - The `infIndex` data member of the `CPICapFloor::arguments` class.
  - A redundant constructor of `SabrSmileSection`.
  - The empty headers
    `ql/experimental/amortizingbonds/amortizingcmsratebond.hpp`,
    `ql/experimental/amortizingbonds/amortizingfixedratebond.hpp` and
    `ql/experimental/amortizingbonds/amortizingfloatingratebond.hpp`.

- Deprecated the constructor of `Currency` and `Currency::Data` taking
  a format string, and the `Currency::format` method.


**Thanks go also** to Yi Jiang (@yjian012), Hoang Giap Vu (@hgv79116)
and Jonathan Sweemer (@sweemer) for smaller fixes and improvements.
