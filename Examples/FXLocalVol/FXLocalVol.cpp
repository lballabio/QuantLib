/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2025 Raghav Subbarao

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file FXLocalVol.cpp
    \brief FX local vol and stochastic local vol (SLV) risk engine example.

    Demonstrates the full workflow for risk-managing an FX vanilla options book:

    PART A — Local Volatility
    1.  Market data: EURUSD pillar quotes (ATM, 25d/10d RR, 25d/10d BF)
        for eleven tenors: O/N, 1W, 2W, 1M, 2M, 3M, 6M, 9M, 1Y, 18M, 2Y.
    2.  Calibrate fxVarianceSurfaceNCP<quadraticSmileSection>.
    3.  Wrap in a GeneralizedBlackScholesProcess.
    4.  Price a 3M EUR call (pillar tenor) under BS-FD and local-vol Dupire FD.
    5.  Full FX Greeks via FxVanillaBumpRisk (sticky-delta convention).
    6.  Sticky-delta vs sticky-strike delta comparison.
    7.  FixedLocalVolSurface spot-check against Dupire.

    PART B — Stochastic Local Volatility (SLV)
    8.  Calibrate a Heston stochastic vol model to the surface
        (5 key tenors x 3 strikes via HestonCalibrator).
    9.  Calibrate the SLV leverage function L(t,S) via the
        Fokker-Planck PDE (HestonSLVLeverageCalibrator).
    10. Price a 4M EUR call — a *non-pillar* tenor between 3M and 6M —
        under both local vol and SLV, and compute spot delta, vega, and
        gamma by bump-and-reval on the SLV engine.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif

// FX vol surface
#include <ql/experimental/fxslv/fxlocalvolsurface.hpp>
#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/fxsettlementforeigncurve.hpp>
#include <ql/experimental/fxslv/fxvariancesurface.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbystrike.hpp>
#include <ql/experimental/fxslv/tradingtimetermstructure.hpp>

// SLV calibration framework
#include <ql/experimental/fxslv/fxslvpricingcontext.hpp>
#include <ql/experimental/fxslv/hestoncalibrator.hpp>
#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>

// Process and pricing
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/experimental/fxslv/fxvanillagreeks.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>

// Instruments
#include <ql/instruments/vanillaoption.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/payoffs.hpp>

// Market data primitives
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/quotes/deltavolquote.hpp>

// Time
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/settings.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace QuantLib;

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

static void printSeparator(int width = 72) {
    std::cout << std::string(width, '=') << "\n";
}

static void printGreeks(const std::string& label, const FxVanillaGreeks& g) {
    const int w0 = 40;
    const int w1 = 14;
    std::cout << "\n  " << label << "\n"
              << "  " << std::string(w0 + w1, '-') << "\n"
              << std::fixed;
    auto row = [&](const std::string& name, Real val, int prec = 2) {
        std::cout << "  " << std::setw(w0) << std::left << name
                  << std::setw(w1) << std::right << std::setprecision(prec) << val
                  << "\n";
    };
    row("NPV (USD)",                              g.npv,       2);
    row("Spot delta  (USD per 1% spot)",          g.spotDelta, 2);
    row("Fwd delta   (USD per 1% fwd)",           g.fwdDelta,  2);
    row("Spot gamma  (USD per 1% spot move)",     g.spotGamma, 2);
    row("Fwd gamma   (USD per 1% fwd move)",      g.fwdGamma,  2);
    row("Theta       (USD/day)",                  g.theta,     2);
    row("Vega        (USD per 1% vol)",           g.vega,      2);
    row("Rega        (USD per 0.1% 1M RR)",       g.rega,      2);
    row("Sega        (USD per 0.01% 1M BF)",      g.sega,      2);
    row("Vanna       (USD per 1%S x 1%vol)",      g.vanna,     2);
    row("Navva       (USD per 1%vol x 1%S) =Vanna",g.navva,   2);
    row("Volga       (USD per 1%vol x 1%vol)",    g.volga,     2);
    std::cout << "  " << std::string(w0 + w1, '-') << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────────────────────────────────────

int main(int, char*[]) {

    try {

        printSeparator();
        std::cout << "  FX Local Vol Risk Engine — EURUSD Example\n";
        printSeparator();

        // ──────────────────────────────────────────────────────────────────────
        //  1. Market data
        // ──────────────────────────────────────────────────────────────────────
        Date today(2, January, 2024);
        Settings::instance().evaluationDate() = today;
        DayCounter dc = Actual365Fixed();

        auto spotSQ = ext::make_shared<SimpleQuote>(1.0850);
        Handle<Quote> spot(spotSQ);

        Handle<YieldTermStructure> eurTs(ext::make_shared<FlatForward>(0, NullCalendar(), 0.0400, dc));
        Handle<YieldTermStructure> usdTs(ext::make_shared<FlatForward>(0, NullCalendar(), 0.0525, dc));
        // the foreign curve giving processes the settlement forward: from the
        // spot date to the delivery date of each expiry, as the surface uses
        Handle<YieldTermStructure> eurFwdTs(ext::make_shared<FxSettlementForeignCurve>(
            eurTs, usdTs, FxSettlementConvention(WeekendsOnly(), 2)));

        Handle<TradingTimeTermStructure> timeTs(ext::make_shared<TradingTimeTermStructure>(today, WeekendsOnly(), 0.0));

        DeltaVolQuote::DeltaType  deltaType = DeltaVolQuote::Fwd;
        DeltaVolQuote::AtmType    atmType   = DeltaVolQuote::AtmFwd;
        FxRrBfQuotes::FlyType   flyType   = FxRrBfQuotes::SmileStrangle;

        // Eleven pillar tenors: O/N through 2Y, generated from the reference date.
        const Calendar calendar = WeekendsOnly();
        const std::vector<Period> tenors = {1*Days, 1*Weeks, 2*Weeks,
                                            1*Months, 2*Months, 3*Months,
                                            6*Months, 9*Months, 1*Years,
                                            18*Months, 2*Years};
        const std::vector<std::string> tenorLabels = {"O/N","1W","2W","1M","2M","3M","6M","9M","1Y","18M","2Y"};

        std::vector<Date> pillars;
        pillars.reserve(tenors.size());
        for (const auto& p : tenors)
            pillars.push_back(calendar.advance(today, p));

        // Market quotes — indicative EURUSD levels as of Jan 2024.
        // ATM vol rises from 6.2% at O/N to 10.0% at 2Y.
        // Negative RR = EUR put skew (standard EURUSD convention).
        // Positive BF = convexity (smile is not flat in strike space).
        struct PillarVols { Real atm, rr25, bf25, rr10, bf10; };
        const std::vector<PillarVols> mkt = {// ATM     RR25    BF25    RR10    BF10
                                             { 0.0620, -0.0020, 0.0002, -0.0040, 0.0005 }, // O/N
                                             { 0.0680, -0.0030, 0.0003, -0.0060, 0.0007 }, // 1W
                                             { 0.0720, -0.0050, 0.0004, -0.0100, 0.0010 }, // 2W
                                             { 0.0780, -0.0120, 0.0040, -0.0250, 0.0090 }, // 1M
                                             { 0.0810, -0.0130, 0.0042, -0.0265, 0.0092 }, // 2M
                                             { 0.0850, -0.0140, 0.0045, -0.0280, 0.0095 }, // 3M
                                             { 0.0900, -0.0160, 0.0050, -0.0310, 0.0105 }, // 6M
                                             { 0.0920, -0.0170, 0.0052, -0.0330, 0.0110 }, // 9M
                                             { 0.0950, -0.0180, 0.0055, -0.0350, 0.0115 }, // 1Y
                                             { 0.0970, -0.0190, 0.0058, -0.0370, 0.0120 }, // 18M
                                             { 0.1000, -0.0200, 0.0060, -0.0390, 0.0125 }, // 2Y
                                            };

        std::vector<Real> deltas = { 0.25, 0.10 };

        // Build all quote handles
        std::vector<Handle<Quote>> atms;
        std::vector<std::vector<Handle<Quote>>> rrs(pillars.size()), bfs(pillars.size());

        // Per-pillar ATM, RR and BF SimpleQuotes for bumping rega/sega
        std::vector<ext::shared_ptr<SimpleQuote>> atmBumps;
        std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> rrBumps(pillars.size());
        std::vector<std::vector<ext::shared_ptr<SimpleQuote>>> bfBumps(pillars.size());
        std::vector<Time> rrPillarTimes(pillars.size()), bfPillarTimes(pillars.size());

        for (Size i = 0; i < pillars.size(); ++i) {
            auto atmBump  = ext::make_shared<SimpleQuote>(mkt[i].atm);
            auto rr25Bump = ext::make_shared<SimpleQuote>(mkt[i].rr25);
            auto rr10Bump = ext::make_shared<SimpleQuote>(mkt[i].rr10);
            auto bf25Bump = ext::make_shared<SimpleQuote>(mkt[i].bf25);
            auto bf10Bump = ext::make_shared<SimpleQuote>(mkt[i].bf10);

            atmBumps.push_back(atmBump);
            rrBumps[i] = {rr25Bump, rr10Bump};
            bfBumps[i] = {bf25Bump, bf10Bump};

            const Time T = dc.yearFraction(today, pillars[i]);
            rrPillarTimes[i] = bfPillarTimes[i] = T;

            atms.push_back(Handle<Quote>(atmBump));
            rrs[i] = {Handle<Quote>(rr25Bump), Handle<Quote>(rr10Bump)};
            bfs[i] = {Handle<Quote>(bf25Bump), Handle<Quote>(bf10Bump)};
        }

        // ──────────────────────────────────────────────────────────────────────
        //  2. Calibrate the FX variance surface
        // ──────────────────────────────────────────────────────────────────────
        auto fxVolSurface = ext::make_shared<fxVarianceSurfaceNCP<FxSabrSmileSection>>(today, spot, pillars, atms, rrs, bfs, deltas,
                                                                                       eurTs, usdTs, timeTs, deltaType, atmType, flyType,
                                                                                       FxSettlementConvention(calendar, 2),
                                                                                       calendar, Following, true);

        fxVolSurface->enableExtrapolation();

        // ──────────────────────────────────────────────────────────────────────
        //  3. Build the GeneralizedBlackScholesProcess
        // ──────────────────────────────────────────────────────────────────────
        auto process = ext::make_shared<GeneralizedBlackScholesProcess>(spot,
                                                                        eurFwdTs,  // dividendYield = foreign (EUR), to the settlement forward
                                                                        usdTs,  // riskFreeRate  = domestic (USD) rate
                                                                        Handle<BlackVolTermStructure>(fxVolSurface));

        // ──────────────────────────────────────────────────────────────────────
        //  4. Define the option: 3M EUR call at ATM forward
        // ──────────────────────────────────────────────────────────────────────
        Date expiryDate = pillars[5]; // 3M pillar (index 5 in the 11-pillar set)
        Time T          = dc.yearFraction(today, expiryDate);
        Real Bd         = usdTs->discount(T);
        Real Bf         = eurFwdTs->discount(T);
        Real fwd        = spot->value() * Bf / Bd;
        Real strike     = fwd; // ATM forward

        auto payoff   = ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);
        auto exercise = ext::make_shared<EuropeanExercise>(expiryDate);
        auto option   = ext::make_shared<VanillaOption>(payoff, exercise);
        Real iv = fxVolSurface->blackVol(T, strike);

        // ──────────────────────────────────────────────────────────────────────
        //  Print market data summary
        // ──────────────────────────────────────────────────────────────────────

        std::cout << std::fixed << std::setprecision(4);
        std::cout << iv << "\n";
        std::cout << "\n  Market data:\n"
                  << "    Reference date  : " << today << "\n"
                  << "    Spot (EURUSD)   : " << spot->value() << "\n"
                  << "    EUR rate (for)  : 4.00%\n"
                  << "    USD rate (dom)  : 5.25%\n"
                  << "    Delta type      : Forward (Fwd)\n"
                  << "    ATM type        : Delta-neutral forward (AtmFwd)\n"
                  << "    Fly type        : Smile strangle\n\n";

        const int wp = 7;
        std::cout << "  Pillar quotes:\n";
        std::cout << "  " << std::setw(8) << "Tenor"
                  << std::setw(wp) << "ATM%"
                  << std::setw(wp) << "25RR%"
                  << std::setw(wp) << "25BF%"
                  << std::setw(wp) << "10RR%"
                  << std::setw(wp) << "10BF%"
                  << "\n"
                  << "  " << std::string(8 + 5 * wp, '-') << "\n";

        for (Size i = 0; i < pillars.size(); ++i) {
            std::cout << "  " << std::setw(8) << tenorLabels[i]
                      << std::setw(wp) << std::setprecision(2) << mkt[i].atm  * 100
                      << std::setw(wp) << mkt[i].rr25 * 100
                      << std::setw(wp) << mkt[i].bf25 * 100
                      << std::setw(wp) << mkt[i].rr10 * 100
                      << std::setw(wp) << mkt[i].bf10 * 100
                      << "\n";
        }

        const Real notional = 1.0e6; // EUR 1,000,000
        std::cout << "\n  Option: 3M EURUSD EUR call / USD put\n"
                  << "    Expiry    : " << expiryDate << "\n"
                  << "    Strike    : " << std::setprecision(4) << strike
                  << " (3M ATM forward)\n"
                  << "    3M fwd    : " << fwd << "\n"
                  << "    Notional  : EUR "
                  << std::fixed << std::setprecision(0) << notional << "\n";

        // ──────────────────────────────────────────────────────────────────────
        //  5. Compute Greeks
        // ──────────────────────────────────────────────────────────────────────
        FxVanillaBumpRisk riskCalc(option, process, spotSQ, atmBumps,
                                   rrBumps, rrPillarTimes,
                                   bfBumps, bfPillarTimes,
                                   notional,
                                   0.001, /*spotBump=*/
                                   0.001, /*volBump=*/
                                   0.001, /*rrBump=*/
                                   0.001, /*bfBump=*/
                                   100, /*tGrid=*/
                                   100 /*xGrid=*/
                                   );
        // The short-dated wings of this surface have calendar arbitrage, so
        // Dupire local vol is not defined everywhere; opt in to a fallback
        // there, and report how often it is used, rather than fail.
        const Volatility localVolFallback = 0.01;
        riskCalc.setLocalVolFallback(localVolFallback);

        FxVanillaGreeks lvGreeks = riskCalc.calculate(/*localVol=*/true);
        std::cout << "  Local vol fallback (" << localVolFallback * 100.0 << "%) used at "
                  << riskCalc.localVolFallbacks() << " points\n";
        FxVanillaGreeks bsGreeks = riskCalc.calculate(/*localVol=*/false);

        // ──────────────────────────────────────────────────────────────────────
        //  Print pricing comparison
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  Pricing comparison  (notional = EUR 1,000,000)\n";
        printSeparator();

        const int wa = 30;
        const int wb = 14;
        std::cout << "\n  " << std::setw(wa) << std::left << "Engine"
                  << std::setw(wb) << std::right << "NPV (USD)"
                  << "\n"
                  << "  " << std::string(wa + wb, '-') << "\n"
                  << "  " << std::setw(wa) << std::left << "Black-Scholes FD"
                  << std::setw(wb) << std::right << std::fixed
                  << std::setprecision(2) << bsGreeks.npv
                  << "\n"
                  << "  " << std::setw(wa) << std::left << "Local vol Dupire FD"
                  << std::setw(wb) << std::right << lvGreeks.npv
                  << "\n"
                  << "  " << std::setw(wa) << std::left << "Difference"
                  << std::setw(wb) << std::right << (lvGreeks.npv - bsGreeks.npv)
                  << "\n";

        // ──────────────────────────────────────────────────────────────────────
        //  Print Greeks
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  Greeks  (notional = EUR 1,000,000)\n";
        printSeparator();

        printGreeks("Black-Scholes FD (sticky-delta)", bsGreeks);
        printGreeks("Local vol Dupire FD (sticky-delta)", lvGreeks);

        std::cout << "\n  Notes on rega/sega time-scaling:\n"
                  << "    Rega: 1M tenor bumped by rrBump=0.1%; other tenors by\n"
                  << "          rrBump * sqrt(T_1M / T_pillar). E.g.:\n"
                  << "          3M: 0.1% * sqrt(1/3) = 0.058%\n"
                  << "          1Y: 0.1% * sqrt(1/12) = 0.029%\n"
                  << "    Sega: same scaling with bfBump=0.1%, reported for 0.01%.\n\n";

        // ──────────────────────────────────────────────────────────────────────
        //  Sticky-delta vs sticky-strike comparison
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  Sticky-delta vs Sticky-strike (Local vol Dupire FD)\n";
        printSeparator();

        FxVanillaGreeks ssGreeks = riskCalc.calculate(/*localVol=*/true,
                                                       FxVanillaBumpRisk::StickyType::Strike);

        printGreeks("Local vol Dupire FD (sticky-delta)", lvGreeks);
        printGreeks("Local vol Dupire FD (sticky-strike)", ssGreeks);

        const int wd = 44, wdv = 12;
        std::cout << "\n  Difference (sticky-delta minus sticky-strike):\n"
                  << "  " << std::string(wd + wdv, '-') << "\n"
                  << std::fixed;
        auto diffRow = [&](const std::string& name, Real a, Real b, int prec = 2) {
            std::cout << "  " << std::setw(wd) << std::left << name
                      << std::setw(wdv) << std::right << std::setprecision(prec)
                      << (a - b) << "\n";
        };
        diffRow("Spot delta (USD per 1% spot)",  lvGreeks.spotDelta, ssGreeks.spotDelta);
        diffRow("Fwd delta  (USD per 1% fwd)",   lvGreeks.fwdDelta,  ssGreeks.fwdDelta);
        diffRow("Spot gamma (USD per 1% spot)",  lvGreeks.spotGamma, ssGreeks.spotGamma);
        diffRow("Vega       (USD per 1% vol)",   lvGreeks.vega,      ssGreeks.vega);
        diffRow("Vanna      (USD per 1%S x 1%vol)", lvGreeks.vanna,  ssGreeks.vanna);
        diffRow("Volga      (USD per 1%vol x 1%vol)", lvGreeks.volga, ssGreeks.volga);
        std::cout << "  " << std::string(wd + wdv, '-') << "\n\n"
                  << "  Non-zero spot delta difference = skew adjustment to delta.\n"
                  << "  With EUR put skew (negative RR), sticky-delta delta is lower\n"
                  << "  because the vol surface falls as spot rises.\n\n";

        // ──────────────────────────────────────────────────────────────────────
        //  6. Local vol surface: Dupire vs FixedLocalVolSurface comparison
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  Local vol surface comparison\n";
        printSeparator();

        std::vector<Time> lvTimes = { 1.0/12, 3.0/12, 6.0/12, 12.0/12 };

        std::vector<Real> lvStrikes;
        for (int i = -3; i <= 3; ++i)
            lvStrikes.push_back(fwd * std::exp(0.05 * i));

        riskCalc.printLocalVolComparison(lvTimes, lvStrikes);

        std::cout << "  D = on-the-fly Dupire local vol\n"
                  << "  F = pre-sampled FixedLocalVolSurface\n"
                  << "  Values in percent (%)\n\n";

        auto fixedLV = riskCalc.buildFixedLocalVolSurface(lvTimes, lvStrikes);
        std::cout << "  FixedLocalVolSurface built with "
                  << lvTimes.size() << " time steps x "
                  << lvStrikes.size() << " strikes.\n"
                  << "  Sample local vol at (T=0.25, K=" << std::fixed
                  << std::setprecision(4) << strike << "): "
                  << std::setprecision(2)
                  << fixedLV->localVol(0.25, strike, true) * 100.0 << "%\n\n";

        // ══════════════════════════════════════════════════════════════════════
        //  PART B — Stochastic Local Volatility
        // ══════════════════════════════════════════════════════════════════════

        // ──────────────────────────────────────────────────────────────────────
        //  7. Build the Dupire local vol surface from the calibrated variance
        //     surface.  It is zero where no variance accrues, e.g. over
        //     weekends in trading time, and fails on arbitrage rather than
        //     substituting a value.
        // ──────────────────────────────────────────────────────────────────────
        auto localVolSurface = ext::make_shared<FxLocalVolSurface>(Handle<BlackVolTermStructure>(fxVolSurface),
                                                                   usdTs, eurFwdTs, spot,
                                                                   1.0e-7, 0.0, localVolFallback);
        localVolSurface->enableExtrapolation();
        Handle<LocalVolTermStructure> localVolHandle(localVolSurface);

        // ──────────────────────────────────────────────────────────────────────
        //  8. Calibrate the Heston stochastic vol model
        //
        //  Use five key tenors (1M, 3M, 6M, 1Y, 2Y) with three strikes each:
        //    K = 0.95F  (OTM put),  K = F  (ATM),  K = 1.05F  (OTM call)
        //  The surface is queried at these absolute strikes to build the
        //  HestonModelHelper instruments.
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  PART B — Stochastic Local Volatility (SLV)\n";
        printSeparator();

        const std::vector<Period> calibTenors = {1*Months, 3*Months, 6*Months, 1*Years, 2*Years};

        std::vector<StochVolCalibrator::Pillar> hestonPillars;
        for (const auto& tenor : calibTenors) {
            const Date   expiry = calendar.advance(today, tenor);
            const Time   Tc     = dc.yearFraction(today, expiry);
            const Real   fwdC   = spotSQ->value() * eurTs->discount(Tc) / usdTs->discount(Tc);
            for (Real m : { 0.95, 1.00, 1.05 })
                hestonPillars.push_back({ tenor, m * fwdC });
        }

        // Initial Heston parameter guess consistent with observed EURUSD vols.
        HestonParams hp;
        hp.v0    = 0.078 * 0.078;   // ≈ (7.8% 1M ATM vol)²
        hp.kappa = 1.50;
        hp.theta = 0.10 * 0.10;     // ≈ (10% long-run vol)²
        hp.sigma = 0.15;
        hp.rho   = -0.25;

        std::cout << "\n  Calibrating Heston model to "
                  << hestonPillars.size() << " instruments "
                  << "(" << calibTenors.size() << " tenors x 3 strikes)...\n";

        // Build the SLV pricing context — this object owns all calibrated state
        // and is the factory for pricing engines.
        Handle<BlackVolTermStructure> volHandle(fxVolSurface);

        FxSLVPricingContext slvCtx(spot, usdTs, eurFwdTs, volHandle, localVolHandle, calendar);

        HestonCalibrator hestonCal(hp);

        // Use a coarser FDM grid than production to keep the example fast.
        // For production use HestonSLVLeverageCalibrator::defaultParams().
        auto fdmParams = HestonSLVLeverageCalibrator::defaultParams();
        fdmParams.xGrid            = 51;
        fdmParams.vGrid            = 21;
        fdmParams.tMaxStepsPerYear = 52;
        fdmParams.tMinStepsPerYear = 12;
        fdmParams.tStepNumberDecay = 2.0;

        HestonSLVLeverageCalibrator levCal(fdmParams);

        const Date slvEndDate = calendar.advance(today, 2*Years);
        slvCtx.calibrate(hestonCal, levCal, hestonPillars, slvEndDate);

        // ── Print Heston calibration results ────────────────────────────────
        const auto heston = slvCtx.hestonModel();

        std::cout << "\n  Calibrated Heston parameters:\n"
                  << std::fixed << std::setprecision(6)
                  << "    kappa (mean reversion speed) : " << heston->kappa() << "\n"
                  << "    theta (long-run variance)    : " << heston->theta()
                  << "  (" << std::setprecision(2)
                  << std::sqrt(heston->theta()) * 100.0 << "% long-run vol)\n"
                  << std::setprecision(6)
                  << "    sigma (vol-of-vol)           : " << heston->sigma() << "\n"
                  << "    rho   (spot-var correlation) : " << heston->rho()   << "\n"
                  << "    v0    (initial variance)     : " << heston->v0()
                  << "  (" << std::setprecision(2)
                  << std::sqrt(heston->v0()) * 100.0 << "% initial vol)\n";

        const Real fellerRatio = 2.0 * heston->kappa() * heston->theta() / (heston->sigma() * heston->sigma());
        std::cout << std::setprecision(4)
                  << "    Feller ratio 2kθ/σ²          : " << fellerRatio
                  << (fellerRatio > 1.0 ? "  (satisfied)\n" : "  (VIOLATED)\n");

        std::cout << "\n  Per-pillar implied-vol errors (model - market):\n";
        const auto& errors = slvCtx.hestonCalibrationErrors();
        Size errIdx = 0;
        for (const auto& tenor : calibTenors) {
            std::cout << "    " << std::setw(4) << std::left;
            // print tenor label
            if      (tenor == 1*Months) std::cout << "1M";
            else if (tenor == 3*Months) std::cout << "3M";
            else if (tenor == 6*Months) std::cout << "6M";
            else if (tenor == 1*Years)  std::cout << "1Y";
            else                        std::cout << "2Y";
            std::cout << " :";
            for (int k = 0; k < 3; ++k, ++errIdx) {
                if (errIdx < errors.size())
                    std::cout << std::setw(8) << std::right << std::fixed
                              << std::setprecision(2)
                              << errors[errIdx] * 100.0 << "%";
            }
            std::cout << "\n";
        }
        std::cout << "\n  RMSE: " << std::setprecision(2)
                  << slvCtx.hestonRmse() * 100.0 << " vol pts\n";

        // ──────────────────────────────────────────────────────────────────────
        //  9. Calibrate the SLV leverage function
        //
        //  The leverage function L(t,S) is solved from the Fokker-Planck PDE
        //  for the joint density p(t,S,v) of the Heston SLV process:
        //
        //     L(t,S)² = σ_local(t,S)² / E[v_t | S_t = S]
        //
        //  The result is stored as a FixedLocalVolSurface on the FDM grid.
        //  Calibration happens inside slvCtx.calibrate() above.
        // ──────────────────────────────────────────────────────────────────────
        std::cout << "\n  Leverage function calibrated on ["
                  << today << ", " << slvEndDate << "].\n";

        const auto leverageFct = slvCtx.leverageFunction();

        // Spot-check: print L(t, F) at a few maturities.
        std::cout << "\n  L(t, spot) at ATM forward (mixing factor = 1.0):\n";
        std::cout << "  " << std::string(36, '-') << "\n"
                  << "  " << std::setw(10) << "Tenor"
                  << std::setw(14) << "L(t, F)"
                  << "\n"
                  << "  " << std::string(36, '-') << "\n";
        for (const auto& tenor : { 1*Months, 3*Months, 6*Months, 1*Years, 2*Years }) {
            const Date   expiryL = calendar.advance(today, tenor);
            const Time   TL      = dc.yearFraction(today, expiryL);
            const Real   fwdL    = spotSQ->value()
                                   * eurTs->discount(TL) / usdTs->discount(TL);
            const Real   L       = leverageFct->localVol(TL, fwdL, true);
            std::string  lbl;
            if      (tenor == 1*Months) lbl = "1M";
            else if (tenor == 3*Months) lbl = "3M";
            else if (tenor == 6*Months) lbl = "6M";
            else if (tenor == 1*Years)  lbl = "1Y";
            else                        lbl = "2Y";
            std::cout << "  " << std::setw(10) << lbl
                      << std::setw(14) << std::setprecision(4) << L
                      << "\n";
        }
        std::cout << "  " << std::string(36, '-') << "\n"
                  << "  (L ≈ 1 means Heston matches local vol exactly at that point)\n";

        // ──────────────────────────────────────────────────────────────────────
        // 10. Price a 4M EUR call — non-pillar tenor between 3M and 6M —
        //     under local vol (Dupire FD) and SLV, then compute Greeks.
        //
        //  Greeks are computed by bump-and-reval on the SLV engine:
        //    Spot delta : (NPV(S+dS) - NPV(S-dS)) / (2 dS)      scaled to per 1% S
        //    Spot gamma : (NPV(S+dS) - 2 NPV + NPV(S-dS)) / dS² scaled to per 1% S
        //    Vega       : (NPV(σ+dσ) - NPV(σ-dσ)) / (2 dσ)      parallel ATM bump
        //
        //  Theta in an SLV model requires re-solving the leverage PDE at a
        //  shifted evaluation date, which is expensive.  For intraday risk
        //  management it is common to use the local-vol theta as a proxy.
        // ──────────────────────────────────────────────────────────────────────
        printSeparator();
        std::cout << "  4M EUR call  —  non-pillar tenor (between 3M and 6M pillars)\n";
        printSeparator();

        const Date   expiry4M = calendar.advance(today, 4*Months);
        const Time   T4M      = dc.yearFraction(today, expiry4M);
        const Real   fwd4M    = spotSQ->value()
                                * eurTs->discount(T4M) / usdTs->discount(T4M);

        auto payoff4M   = ext::make_shared<PlainVanillaPayoff>(Option::Call, fwd4M);
        auto exercise4M = ext::make_shared<EuropeanExercise>(expiry4M);
        auto call4M     = ext::make_shared<VanillaOption>(payoff4M, exercise4M);

        std::cout << "\n    Expiry   : " << expiry4M << "\n"
                  << "    Strike   : " << std::setprecision(4) << fwd4M
                  << " (ATM forward)\n"
                  << "    4M fwd   : " << fwd4M << "\n"
                  << "    Notional : EUR "
                  << std::fixed << std::setprecision(0) << notional << "\n";

        // ── Local vol baseline price ─────────────────────────────────────────
        auto gbsProcess4M = ext::make_shared<GeneralizedBlackScholesProcess>(spot, eurFwdTs, usdTs,
                                                                             volHandle, localVolHandle);
        auto call4M_lv = ext::make_shared<VanillaOption>(payoff4M, exercise4M);
        call4M_lv->setPricingEngine(MakeFdBlackScholesVanillaEngine(gbsProcess4M)
                                        .withTGrid(100)
                                        .withXGrid(100)
                                        .withLocalVol(true));
        const Real lvNpv4M = call4M_lv->NPV() * notional;

        // ── SLV price ────────────────────────────────────────────────────────
        const FdmGridConfig grid{ /*tGrid=*/100, /*xGrid=*/100,
                                  /*vGrid=*/50, /*dampingSteps=*/0 };
        call4M->setPricingEngine(slvCtx.vanillaEngine(grid));
        const Real slvNpv4M = call4M->NPV() * notional;

        // ── Spot bump parameters ─────────────────────────────────────────────
        const Real S0  = spotSQ->value();
        const Real dS  = S0 * 0.001;   // 0.1% absolute spot bump

        spotSQ->setValue(S0 + dS);
        const Real slvUp = call4M->NPV() * notional;
        spotSQ->setValue(S0 - dS);
        const Real slvDn = call4M->NPV() * notional;
        spotSQ->setValue(S0);

        // Scale to "per 1% spot move"
        const Real pctS      = 0.01 * S0;
        const Real slvDelta  = (slvUp - slvDn)  / (2.0 * dS) * pctS;
        const Real slvGamma  = (slvUp - 2.0 * slvNpv4M + slvDn) / (dS * dS) * pctS * pctS;

        // ── Vega: parallel bump of all ATM vol quotes ─────────────────────────
        const Real dVol = 0.001;  // 10 bp absolute
        for (auto& sq : atmBumps) sq->setValue(sq->value() + dVol);
        const Real slvVolUp = call4M->NPV() * notional;
        for (auto& sq : atmBumps) sq->setValue(sq->value() - 2.0 * dVol);
        const Real slvVolDn = call4M->NPV() * notional;
        for (auto& sq : atmBumps) sq->setValue(sq->value() + dVol);  // restore

        // Scale to "per 1% (100bp) vol move"
        const Real slvVega = (slvVolUp - slvVolDn) / (2.0 * dVol) * 0.01;

        // ── Local vol theta (proxy) ───────────────────────────────────────────
        // Advance evaluation date by one calendar day using the LV engine;
        // re-calibrating the full SLV model just for theta is impractical
        // in an interactive example.
        Settings::instance().evaluationDate() = today + 1;
        const Real lvNpvTomorrow = call4M_lv->NPV() * notional;
        Settings::instance().evaluationDate() = today;
        const Real lvTheta4M = lvNpvTomorrow - lvNpv4M;  // USD per calendar day

        // ── Print results ─────────────────────────────────────────────────────
        const int wl = 44, wv = 14;
        std::cout << "\n  Pricing:\n"
                  << "  " << std::string(wl + wv, '-') << "\n";
        auto row = [&](const std::string& label, Real val, int prec = 2) {
            std::cout << "  " << std::setw(wl) << std::left << label
                      << std::setw(wv) << std::right << std::fixed
                      << std::setprecision(prec) << val << "\n";
        };
        row("Local vol Dupire FD NPV (USD)", lvNpv4M);
        row("SLV (Heston + leverage) NPV (USD)", slvNpv4M);
        row("Difference  SLV - LV  (USD)", slvNpv4M - lvNpv4M);
        const Real relDiff = std::fabs(slvNpv4M - lvNpv4M)
                             / std::max(lvNpv4M, 1.0) * 100.0;
        row("Relative difference (%)", relDiff);
        std::cout << "  " << std::string(wl + wv, '-') << "\n";

        std::cout << "\n  SLV Greeks  (notional = EUR 1,000,000):\n"
                  << "  " << std::string(wl + wv, '-') << "\n";
        row("Spot delta  (USD per 1% spot move)", slvDelta);
        row("Spot gamma  (USD per (1% spot)²)",   slvGamma);
        row("Vega        (USD per 1% vol move)",  slvVega);
        row("Theta       (USD/day, LV proxy)",    lvTheta4M);
        std::cout << "  " << std::string(wl + wv, '-') << "\n";

        std::cout << "\n  Note: Theta shown is the local-vol proxy.  Computing\n"
                  << "  SLV theta exactly requires re-calibrating the leverage\n"
                  << "  function at the shifted date — typically done overnight.\n\n";

        std::cout << "  Local vol fallback used at " << localVolSurface->fallbacks()
                  << " points in the SLV and local-vol pricing above.\n\n";

        printSeparator();
        std::cout << "  Done.\n";
        printSeparator();

        return 0;

    } catch (std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\nUnknown error\n";
        return 1;
    }
}
