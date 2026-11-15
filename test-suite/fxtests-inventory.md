# FX test inventory

A working document for rationalising the fork's FX tests, one suite at a
time.  Each suite gets a table of its tests (what each asserts, the kind of
check, its tolerance and the basis for it, overlaps) and a list of findings
and proposals.  Decisions go in the last column as they are made.

Kinds of check:

- **identity**: an exact relation the code must satisfy (parity, a round
  trip, an algebraic equivalence); tolerance at rounding level.
- **reference**: values from an independent implementation or closed form.
- **FD**: an analytic derivative against a finite difference; the
  tolerance is set by the difference step.
- **regression**: a limit a little above today's number, e.g. a fit error;
  it catches large regressions only, and says nothing about correctness.
- **contract**: an error or notification the interface promises.
- **smoke**: runs and gives a sane sign or range.

## 1. Smile sections (`test-suite/fxsmilesections.cpp`)

36 test cases, about 0.16 s in all: run time is no constraint.

The shared `MarketData` is one market: 1Y expiry, spot 1.7554, rates 5%
foreign and 3% domestic, ATM 14.48%, 25D/10D RR +2.77%/+4.88% (a call
skew), BF 0.74%/2.38%, **premium-adjusted spot deltas with an ATM-forward
ATM**, smile strangles.

### 1.1 Tests

| # | Test | Asserts | Kind | Tolerance (basis) | Overlaps | Proposal | Decision |
|---|---|---|---|---|---|---|---|
| 1 | `testPolynomialSmileSection` | fit RMS; put strike↔delta↔vol consistency | regression; identity | 15bp; 1e-6 | consistency ⊂ #18 | fold into model table (§1.3) | |
| 2 | `testSabrSmileSection` | fit RMS; consistency; parameter accessors calibrate lazily; α>0, ν>0, \|ρ\|<1 | regression; identity; contract | 25bp; 1e-6 | consistency ⊂ #18; bounds ⊂ #23 | keep the lazy-accessor check, fold the rest | |
| 3 | `testSviSmileSection` | exact fit (5 params, 5 quotes); consistency | identity | 1e-6 | consistency ⊂ #18 | fold; keep "exact fit" as a model property | |
| 4 | `testQuadraticSmileSection` | fit RMS; consistency | regression; identity | 30bp; 1e-5 | ⊂ #18 | fold | |
| 5 | `testCostSmileSectionFlatDynamics` | fit RMS; consistency | regression; identity | 17bp; 1e-5 | ⊂ #18 | fold | |
| 6 | `testCostSmileSectionScaledDynamics` | fit RMS; consistency | regression; identity | 35bp; 1e-5 | ⊂ #18 | fold | |
| 7 | `testStickyDeltaSmileSolver` | first-order sticky-delta smile against Python RK4; singular point in closed form; slope = smile slope; slope solves the cost equation; PA ≡ Cθ+Cη; scale invariance; `hasCosts`; bad costs throw | reference; identity; FD; contract | 1e-9 (reference to 12 d.p.); 1e-12; 1e-6 (FD h=1e-5) | — | keep; strongest test in the file | |
| 8 | `testCostSmileSectionStickyDelta` | fit RMS; consistency; `slope`, `volDerivative` against FD (Spot, PaSpot) | regression; identity; FD | 27bp; 1e-6; 1e-6 | slope/derivative block duplicated in #11 | shared helper; fold fit into model table | |
| 9 | `testStickyDeltaConvexSmileSolver` | convex smile against Python `solve_bvp`; slope and convexity against FD; PA and scale identities; end conditions decay; tolerance below rounding converges; bad costs throw | reference; identity; FD; contract | 1e-10/1e-7/1e-6 (reference to 12/9/8 d.p.); 2e-4 (second FD) | — | keep | |
| 10 | `testCostSmileSectionStickyDeltaConvexSteepSkew` | 3M USDJPY (PaSpot, DNS): costs against Python; fit; no failure when trial smiles run away | reference; regression | 1e-3 to 5e-3 %; 6bp | — | keep; the only real-market case | |
| 11 | `testCostSmileSectionStickyDeltaConvex` | fit; consistency; slope/derivative FD; convexity ≠ 0, flat model's = 0 | regression; identity; FD; smoke | 25bp; 1e-6; `\|κ\|>0` | #8 | shared helper; `\|κ\|>0` is weak, #9 already checks κ against its reference | |
| 12 | `testAtmVolForwardSensitivity` | cost models: ratio of costs; SABR: Hagan's ρν·∂σ/∂α; polynomial: 0 | identity (restates the implementation); reference | 1e-12; 0.5% | — | strengthen: compare with the ATM vol move of the recalibrated smile under a forward bump; add the sticky-delta models | |
| 13 | `testDeltaVolQuoteConstructorPath` | RR/BF quotes and the equivalent delta-vol quotes give the same smile (6 models) | identity | 1e-7 fit, 1e-5 vol (LM stops on relative change) | — | add the two sticky-delta models (model table) | |
| 14 | `testCalibrationRoundTrip` | quotes read off each model's smile are refitted, same smile (8 models, default conventions) | identity | 1e-8 (3e-8 convex) | ⊂ #18 except the sticky-delta models | delete once #18 covers all 8 models | |
| 15 | `testAtmLevelIsTheForward` | `atmLevel()` is the forward under DNS; `SmileSection::optionPrice` is Black's | identity | 1e-12; 1e-10 % | — | keep (only test of the `SmileSection` base interface); fix the stale "8. Market data reactivity" banner above it | |
| 16 | `testPremiumAdjustedMarketStrangles` | market strangle, one delta level, PaSpot/PaFwd, 3 models: exact fit, RR, ATM; same smile after a spot round trip | identity | 1e-8; 1e-10 % | #35 (same feature, Spot); #32 (history) | merge with #35 into one table over delta types × models | |
| 17 | `testMaxCallDelta` | PA call-delta peak of a smile; spot/forward limits; peak above the forward at high total vol | identity; contract | 1e-14; 1e-10 | #19 (converter-level peak), #21 | keep; group with #19 and #21 | |
| 18 | `testRoundTripsInAllConventions` | 6 models × {Spot, Fwd, PaSpot, PaFwd} × {AtmFwd, DNS}: ATM vol on the smile; strike↔delta↔vol; refit own quotes | identity | 1e-10; 1e-8; 1e-8, 1e-6 | supersedes the consistency checks of #1–#6, #8, #11 and #14 | the core test: extend to all 8 models and AtmSpot | |
| 19 | `testDeltaConverter` | `FxDeltaConverter` against `BlackDeltaCalculator`; parity; strike round trips; ATM strikes; PA peak; ITM PA puts beyond −B_f | reference; identity; contract | 1e-15; 1e-14; 1e-10/1e-8 (inverse normal) | #17 | keep; a delta-conventions group | |
| 20 | `testStrikeByDeltaOnlyVisitsStrikesNearTheRoot` | inversion works on a smile defined only near the money | contract | 1e-8 | — | keep | |
| 21 | `testPremiumAdjustedCallsBeyondTheFlatVolPeak` | PA call deltas the smile reaches but a flat vol does not | contract (regression for a past bug) | 1e-8 | #17 | keep; group with #17, #19 | |
| 22 | `testQuarticRoots` | real roots of `QuarticPolynomial` | reference | 1e-10 | — | keep; **add `CubicPolynomial`** (open issue #19) | |
| 23 | `testSviAndSabrParametersStayValid` | 3 quote sets × {Spot, PaSpot} × {1Y, 5Y}: SVI b>0, \|ρ\|≤1, σ>0, w_min>0, b(1+\|ρ\|)≤2; SABR bounds | contract | — | #2 bounds | keep; valid parameters are not absence of arbitrage (§1.2) | |
| 24 | `testMarketDataReactivity` | each quote and spot moves a polynomial smile and back; an ATM bump moves the ATM vol by about as much | contract | 1e-6 move; 1e-8 %; 1e-3 | — | run over all models; add the curves | |
| 25 | `testSettlementConventionDates` | `FxSettleConvention` value dates | reference | exact | `testFxTenorExpiryDates` in fxlocalvol.cpp | move both to a conventions group | |
| 26 | `testDateModeForwardUsesSpotToDelivery` | forward from spot to delivery; vol time from trade date | reference | 1e-12 % | — | keep; a forward/settlement group | |
| 27 | `testDateModeIgnoresCurveReferenceDate` | forward independent of curves' reference dates; curve starting after spot throws | identity; contract | 1e-12 % | — | keep; same group | |
| 28 | `testDateModeFloatsWithEvaluationDate` | spot date and forward follow the evaluation date; a fixed reference date does not | reference | 1e-12 % | — | keep; same group | |
| 29 | `testTimeModeForwardAndConsistencyChecks` | time-mode forward; curves must share a time axis | reference; contract | 1e-12 % | — | keep; same group | |
| 30 | `testSmileQuotesValidationAndNotification` | quote-object input validation; sections notified through a shared quote object | contract | — | — | keep; a quotes/calibration-contract group | |
| 31 | `testDeltaVolQuotesUseTheirOwnConventions` | each delta-vol quote is read in its own delta convention | identity | 1e-8 | — | keep; same group | |
| 32 | `testCalibrationDoesNotDependOnHistory` | same quotes give the same smile after recalibrating elsewhere (polynomial) | identity | 1e-10 % | #16 | run over all models (model table) | |
| 33 | `testFailedCalibrationThrows` | a NaN target fails calibration loudly | contract | — | — | keep; contract group | |
| 34 | `testSmileQuotesMustFitDuringCalibration` | quotes may only fit a section while calibrating it | contract | — | — | keep; contract group | |
| 35 | `testMarketStrangleCalibration` | market strangles, Spot/DNS, quadratic: strangle repriced, RR, ATM; two levels by least squares; cost models throw off-smile | identity; contract | 1e-8 | #16 | merge with #16 | |
| 36 | `testProbabilitySpaceFunctions` | flat smile: exercise probability and normed call price are Black's; inverse of the probability | reference; identity | 1e-10, 1e-12 | — | keep | |

### 1.2 Findings

1. **One market.** Apart from #10 (3M USDJPY) and a few synthetic quote
   sets (#21, #23), everything runs on one 1Y market, with a call skew and
   an unusual pairing: premium-adjusted deltas with an ATM-forward ATM
   (premium-adjusted pairs normally quote a delta-neutral ATM).  Not
   covered:
   - short expiries (O/N to 1W), where the numerics are hardest;
   - a put skew in the default market;
   - high-vol EM pairs: the USDZAR case that failed the convex calibration
     earlier (spot 16.4576) has no C++ regression test;
   - long-dated premium-adjusted forward deltas (PaFwd beyond 1Y).
2. **No test of no-arbitrage.** No model is checked for a non-negative
   density or call prices decreasing and convex in strike.  #23 checks
   that parameters are valid, which is weaker.  This matters downstream:
   Dupire fails with "butterfly arbitrage" where a smile has negative
   density.
3. **Fit errors are only checked against regression limits.**  The limits
   (15–35bp RMS) were set "a little above the current fit".  They catch a
   broken calibration but not a degraded one; the headroom over today's
   fit is in §1.4.
4. **The model list is written out four times** (#13, #14, #18 and the
   per-model tests), and has drifted: the two sticky-delta models are
   missing from #13 and #18.
5. **#12 checks the formulas, not the dynamics.**  For the cost models it
   compares the implementation with its own formula.  The test that
   matters for greeks is whether the recalibrated smile's ATM vol moves as
   predicted when the forward is bumped.
6. **Gaps in smaller things:** `CubicPolynomial` untested; reactivity
   (#24) only on the polynomial and only to quotes and spot, not curves;
   history independence (#32) only on the polynomial.
7. **Stale banners:** the numbered section comments (1–8) no longer match;
   "8. Market data reactivity" heads `testAtmLevelIsTheForward`.

### 1.3 Proposed structure

Keep one file but group tests into nested suites, which can be run on
their own (`--run_test=QuantLibTests/FxSmileSectionTests/DeltaConventions`)
and need no build-system changes:

| Group | Tests |
|---|---|
| `DeltaConventions` | #19, #17, #21, #20 |
| `QuotesAndCalibration` | #30, #31, #33, #34, #13, #16+#35 |
| `Models` | table-driven over the model registry: fit (#1–#6, #8, #11 fit parts), round trips and refit (#18, absorbing #14), reactivity (#24), history (#32), no-arbitrage (new) |
| `StickyDeltaSmiles` | #7, #9, #10, slope/convexity helper (#8, #11) |
| `Dynamics` | #12 (strengthened) |
| `ForwardAndSettlement` | #26–#29, #25 (or move #25 to a conventions suite with `testFxTenorExpiryDates`) |
| `Utilities` | #22 (+ cubic), #36, #15 |

The design change that does the most is a **model registry**: one table
of `{name, factory, properties}` that every model-level test loops over.

```cpp
struct ModelCase {
    std::string name;
    SectionFactory make;          // as sectionFactory<Section>(md)
    Real fitLimit;                // regression limit on the RMS fit, in vol
    bool exactFit;                // as many parameters as quotes
    bool closedFormOffSmile;      // false for the cost models
    bool hasDynamics;             // atmVolForwardSensitivity() != 0
};
std::vector<ModelCase> models(const MarketData& md);
```

A new model is then one row, and it cannot be left out of a property by
accident (finding 4).  Combined with a small set of named markets
(`MarketData::gbpusd1Y()`, `usdjpy3M()`, `usdzar1M()`, ...), the model
tests become models × markets × conventions, which answers finding 1
without writing new tests.

### 1.4 Fit-error headroom

RMS fit error in vol to the five quotes of `MarketData`, with the limit
the test applies.

| Model | Delta type | Today's fit | Limit | Limit / fit |
|---|---|---|---|---|
| Polynomial | PaSpot | 11.8bp | 15bp | 1.27 |
| SABR | PaSpot | 18.5bp | 25bp | 1.35 |
| SVI | PaSpot | 2e-15 | 1e-6 | (exact fit) |
| Quadratic | PaSpot | 24.8bp | 30bp | 1.21 |
| Cost, flat dynamics | PaSpot | 13.7bp | 17bp | 1.24 |
| Cost, scaled dynamics | PaSpot | 28.4bp | 35bp | 1.23 |
| Cost, sticky delta | PaSpot | 25.5bp | 27bp | 1.06 |
| Cost, sticky delta | Spot | 21.6bp | 27bp | 1.25 |
| Cost, sticky delta convex | PaSpot | 24.0bp | 25bp | 1.04 |
| Cost, sticky delta convex | Spot | 20.1bp | 25bp | 1.24 |

Two things to decide:

- **The limits are loose relative to the fit, and the fits themselves are
  poor.**  Every three-parameter model misses five quotes by 12–28bp RMS,
  which is wider than a typical 25D bid-ask, and the limits let that
  degrade by another 20–35% unnoticed.  Whether that is acceptable is a
  modelling question (three parameters cannot fit two risk reversals and
  two butterflies on this smile); the test cannot answer it, but it
  should at least pin the fit tightly, e.g. at 1.05× today's, so that a
  change in the fit is a deliberate act.
- **The RMS hides where the miss is.**  Checking the residual per quote
  (ATM, 25D, 10D) would show whether the models miss the 10D wings, which
  is what Dupire and SLV are most sensitive to.
