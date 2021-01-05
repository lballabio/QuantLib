Changes for QuantLib 1.21:
==========================

QuantLib 1.21 includes 24 pull requests from several contributors.

The most notable changes are included below.
A detailed list of changes is available in ChangeLog.txt and at
<https://github.com/lballabio/QuantLib/milestone/17?closed=1>.

Portability
-----------

- As previously announced, this is the last release to support Visual
  C++ 2012.  Starting from next release, VC++ 2013 or later will be
  required in order to enable use of C++11 features.

Instruments
-----------

- Improve date generation for CDS schedules under the post-big-bang
  rules (thanks to Francis Duffy).

- Amortizing fixed-rate bonds can now use a generic `InterestRate`
  object (thanks to Piter Dias).

- Added Monte Carlo pricer for discrete-average arithmetic Asian
  options under the Heston model (thanks to Jack Gillett).

- Added analytic and Monte Carlo pricers for discrete-average
  geometric Asian options under the Heston model (thanks to Jack
  Gillett).  Together, they can also be used as a control variate in
  Monte Carlo models for arithmetic Asian options.

- Added analytic pricer for continuous-average geometric Asian
  options under the Heston model (thanks to Jack Gillett).

- Added analytic pricer for forward options under the Heston model
  (thanks to Jack Gillett).

- Added Monte Carlo pricers for forward options under the
  Black-Scholes and the Heston models (thanks to Jack Gillett).

Term structures
---------------

- Added Dutch regulatory term structure, a.k.a. ultimate forward term
  structure (thanks to Marcin Rybacki).

- Generalized exponential spline fitting to an arbitrary number of
  parameters; it is now also possible to fix kappa (thanks to David
  Sansom).

- Fixed averaging period for 1-month SOFR futures rate helper (thanks
  to Eisuke Tani).

Date/time
---------

- Fixed a bug and added 2017 holidays in Thailand calendar (thanks to
  GitHub user `phil-zxx` for the heads-up).

- Updated Chinese calendar for 2021 (thanks to Cheng Li).

- Updated Japanese calendar for 2021 (thanks to Eisuke Tani).


Thanks go also to Francois Botha, Peter Caspers, Ralf Konrad, Matthias
Siemering, Klaus Spanderen and Joseph Wang for smaller fixes,
enhancements and bug reports.
