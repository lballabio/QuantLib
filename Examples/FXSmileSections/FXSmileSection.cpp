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

/*! \file FXSmileSection.cpp
    \brief Example showing calibration of all FX smile section types.

    Demonstrates how to construct and calibrate the following concrete
    fxSmileSection subclasses against a common set of FX volatility market
    quotes (ATM, 25-delta and 10-delta risk reversals and broker flies):

      Strike-parameterised:
        - polynomialSmileSection      (quadratic-in-log-moneyness, 3 params)
        - fxSabrSmileSection          (SABR beta=1, 3 params: alpha, nu, rho)
        - fxSviSmileSection           (SVI raw, 5 params: a, b, rho, m, sigma)
        - fxCostSmileSectionFlatDynamics   (cost model, flat smile dynamics)
        - fxCostSmileSectionScaledDynamics (cost model, scaled smile dynamics)

      Delta-parameterised:
        - quadraticSmileSection       (quadratic in put delta, 3 params)

    For each model the example prints a row in a table showing:
      - the calibrated ATM vol (should recover the input)
      - 25D put / call vols
      - 10D put / call vols
      - implied risk reversals and butterflies for both delta tenors
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif
#include <ql/experimental/fxslv/fxsmilesection.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbystrike.hpp>
#include <ql/experimental/fxslv/fxsmilesectionbydelta.hpp>
#include <ql/experimental/fxslv/fxcostsmilesection.hpp>
#include <ql/quotes/deltavolquote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/fxsmilequotes.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/date.hpp>
#include <ql/settings.hpp>

#include <iomanip>
#include <iostream>
#include <string>

using namespace QuantLib;

// ---------------------------------------------------------------------------
//  Helper: print one result row for a calibrated smile section
// ---------------------------------------------------------------------------

void printRow(const std::string& name,
              FxSmileSection& ss,
              Real market_atm,
              Real market_rr25,
              Real market_bf25,
              Real market_rr10,
              Real market_bf10) {

    const int w0 = 34; // model name column
    const int w  =  9; // vol columns

    Real atm    = ss.volByStrike(ss.atmStrike());
    Real v25c   = ss.volByDelta( 0.25, Option::Call);
    Real v25p   = ss.volByDelta(-0.25, Option::Put);
    Real v10c   = ss.volByDelta( 0.10, Option::Call);
    Real v10p   = ss.volByDelta(-0.10, Option::Put);
    Real rr25   = v25c - v25p;
    Real bf25   = 0.5 * (v25c + v25p) - atm;
    Real rr10   = v10c - v10p;
    Real bf10   = 0.5 * (v10c + v10p) - atm;

    std::cout << std::setw(w0) << std::left  << name
              << std::fixed    << std::setprecision(4)
              << std::setw(w)  << std::right  << atm  * 100.0
              << std::setw(w)  << std::right  << v25p * 100.0
              << std::setw(w)  << std::right  << v25c * 100.0
              << std::setw(w)  << std::right  << v10p * 100.0
              << std::setw(w)  << std::right  << v10c * 100.0
              << std::setw(w)  << std::right  << rr25 * 100.0
              << std::setw(w)  << std::right  << bf25 * 100.0
              << std::setw(w)  << std::right  << rr10 * 100.0
              << std::setw(w)  << std::right  << bf10 * 100.0
              << "\n";

    // Print residuals versus market on the next line
    std::cout << std::setw(w0) << std::left << "  residuals vs market"
              << std::fixed << std::setprecision(5)
              << std::setw(w) << std::right << (atm  - market_atm)  * 100.0
              << std::setw(w) << std::right << ""   // 25p — no direct quote
              << std::setw(w) << std::right << ""   // 25c — no direct quote
              << std::setw(w) << std::right << ""   // 10p
              << std::setw(w) << std::right << ""   // 10c
              << std::setw(w) << std::right << (rr25 - market_rr25) * 100.0
              << std::setw(w) << std::right << (bf25 - market_bf25) * 100.0
              << std::setw(w) << std::right << (rr10 - market_rr10) * 100.0
              << std::setw(w) << std::right << (bf10 - market_bf10) * 100.0
              << "\n\n";
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------

int main(int, char*[]) {

    try {

        std::cout << "\n"
                  << "=============================================================\n"
                  << "  FX Smile Section Calibration Example\n"
                  << "=============================================================\n\n";

        // ------------------------------------------------------------------
        //  Market data
        //
        //  Currency pair: CCY1/CCY2
        //  Spot:          1.7554
        //  Expiry:        1 year (2024-01-02 to 2025-01-02)
        //  Foreign rate:  5% flat (CCY1 risk-free)
        //  Domestic rate: 3% flat (CCY2 risk-free)
        //
        //  Volatility quotes (smile-strangle convention):
        //    ATM (delta-neutral fwd):  14.483%
        //    25D risk reversal:         5.770%
        //    25D broker fly:            0.7425%
        //    10D risk reversal:        10.1575%
        //    10D broker fly:            1.6125%
        //
        //  Delta convention: premium-adjusted spot delta (PaSpot)
        //  ATM convention:   delta-neutral forward (AtmFwd)
        // ------------------------------------------------------------------

        Date todaysDate(2, January, 2024);
        Date expiryDate(2, January, 2025);
        Settings::instance().evaluationDate() = todaysDate;

        Handle<YieldTermStructure> forDiscount(
            ext::make_shared<FlatForward>(todaysDate, 0.05, Actual365Fixed()));
        Handle<YieldTermStructure> domDiscount(
            ext::make_shared<FlatForward>(todaysDate, 0.03, Actual365Fixed()));

        Handle<Quote> spot  = makeQuoteHandle(1.7554);
        Handle<Quote> v_atm = makeQuoteHandle(0.069);

        // Risk-reversal and broker-fly quotes at two delta tenors
        // 10D risk reversal about 1.8x and 10D fly about 3.2x the 25D ones,
        // as is typical of FX smiles
        std::vector<Handle<Quote>> rrs = { makeQuoteHandle(0.0079),
                                           makeQuoteHandle(0.0158) };
        std::vector<Handle<Quote>> bfs = { makeQuoteHandle(0.0030),
                                           makeQuoteHandle(0.01005) };
        std::vector<Real> deltas = { 0.25, 0.10 };

        DeltaVolQuote::DeltaType deltaType = DeltaVolQuote::PaSpot;
        DeltaVolQuote::AtmType   atmType   = DeltaVolQuote::AtmFwd;
        FxRrBfQuotes::FlyType  flyType   = FxRrBfQuotes::SmileStrangle;
        DayCounter               dc        = Actual365Fixed();

        // Settlement: generic pair, weekends-only calendar, T+2 spot lag.
        // For a real pair use the joint calendar of both currencies (and USD
        // if the pair settles through USD) and the pair's spot lag.
        FxSettleConvention   settlement(WeekendsOnly(), 2);

        // Market quotes, shared by all the sections below
        auto smileQuotes = ext::make_shared<FxRrBfQuotes>(v_atm, rrs, bfs, deltas, flyType);

        // Derived market quotes for reference
        Real mkt_atm  = v_atm->value();
        Real mkt_rr25 = rrs[0]->value();
        Real mkt_bf25 = bfs[0]->value();
        Real mkt_rr10 = rrs[1]->value();
        Real mkt_bf10 = bfs[1]->value();

        // ------------------------------------------------------------------
        //  Print market data summary
        // ------------------------------------------------------------------

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "  Reference date : " << todaysDate << "\n"
                  << "  Expiry date    : " << expiryDate << "\n"
                  << "  Spot           : " << spot->value()  << "\n"
                  << "  Foreign rate   : " << forDiscount->zeroRate(1.0, Continuous).rate() * 100.0 << "%\n"
                  << "  Domestic rate  : " << domDiscount->zeroRate(1.0, Continuous).rate() * 100.0 << "%\n"
                  << "\n"
                  << "  Vol quotes (smile-strangle convention):\n"
                  << "    ATM (delta-neutral fwd)  : " << mkt_atm  * 100.0 << "%\n"
                  << "    25D risk reversal        : " << mkt_rr25 * 100.0 << "%\n"
                  << "    25D broker fly           : " << mkt_bf25 * 100.0 << "%\n"
                  << "    10D risk reversal        : " << mkt_rr10 * 100.0 << "%\n"
                  << "    10D broker fly           : " << mkt_bf10 * 100.0 << "%\n"
                  << "\n"
                  << "  Delta convention : premium-adjusted spot (PaSpot)\n"
                  << "  ATM convention   : delta-neutral forward (AtmFwd)\n\n";

        // ------------------------------------------------------------------
        //  Column header
        // ------------------------------------------------------------------

        const int w0 = 34;
        const int w  =  9;
        const std::string sep(w0 + 9 * w, '-');

        std::cout << sep << "\n";
        std::cout << std::setw(w0) << std::left  << "Model"
                  << std::setw(w)  << std::right << "ATM%"
                  << std::setw(w)  << std::right << "25P%"
                  << std::setw(w)  << std::right << "25C%"
                  << std::setw(w)  << std::right << "10P%"
                  << std::setw(w)  << std::right << "10C%"
                  << std::setw(w)  << std::right << "RR25%"
                  << std::setw(w)  << std::right << "BF25%"
                  << std::setw(w)  << std::right << "RR10%"
                  << std::setw(w)  << std::right << "BF10%"
                  << "\n";
        std::cout << sep << "\n\n";

        // ------------------------------------------------------------------
        //  1. Polynomial smile section (strike-parameterised, 3 params)
        //
        //     vol(K) = a*(ln K/F)^2 + b*(ln K/F) + c
        //     Three parameters: (a, b, c)
        //     Five calibration targets => over-determined, best-fit via LM.
        // ------------------------------------------------------------------

        std::cout << "--- Strike-parameterised models ---\n\n";

        {
            FxPolynomialSmileSection ss(expiryDate, spot, smileQuotes,
                                        forDiscount, domDiscount,
                                        deltaType, atmType, dc, settlement);
            printRow("Polynomial (3 params)", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);
        }

        // ------------------------------------------------------------------
        //  2. FX SABR smile section (strike-parameterised, 3 params)
        //
        //     Standard SABR approximation with beta fixed to 1 (log-normal
        //     backbone).  Free parameters: alpha (initial vol), nu (vol-of-vol),
        //     rho (correlation between spot and vol).
        //     Five calibration targets => over-determined, best-fit via LM.
        // ------------------------------------------------------------------

        {
            FxSabrSmileSection ss(expiryDate, spot, smileQuotes,
                                  forDiscount, domDiscount,
                                  deltaType, atmType, dc, settlement);
            printRow("SABR beta=1 (3 params)", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);

            std::cout << "    Calibrated params: alpha=" << std::setprecision(6) << ss.alpha()
                      << "  nu=" << ss.nu()
                      << "  rho=" << ss.rho() << "\n\n";
        }

        // ------------------------------------------------------------------
        //  3. FX SVI smile section (strike-parameterised, 5 params)
        //
        //     SVI raw parameterisation in total implied variance:
        //       w(k) = a + b*(rho*(k-m) + sqrt((k-m)^2 + sigma^2))
        //     where k = ln(K/F) is log-moneyness.
        //     Five parameters exactly match the five calibration targets.
        // ------------------------------------------------------------------

        {
            FxSviSmileSection ss(expiryDate, spot, smileQuotes,
                                 forDiscount, domDiscount,
                                 deltaType, atmType, dc, settlement);
            printRow("SVI raw (5 params)", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);

            std::cout << "    Calibrated params: a=" << std::setprecision(6) << ss.a()
                      << "  b=" << ss.b()
                      << "  rho=" << ss.rho()
                      << "  m=" << ss.m()
                      << "  sigma=" << ss.sigma() << "\n\n";
        }

        // ------------------------------------------------------------------
        //  4. Cost smile section — flat dynamics (strike-parameterised)
        //
        //     Parameterises the smile in terms of option prices (costs) rather
        //     than implied vols.  The flat-dynamics variant assumes the absolute
        //     smile shape does not move with spot.  Calibrates exactly to all
        //     market quotes.
        // ------------------------------------------------------------------

        {
            FxCostSmileSectionFlatDynamics ss(expiryDate, spot, smileQuotes,
                                              forDiscount, domDiscount,
                                              deltaType, atmType,
                                              dc, settlement, Date(), true);
            printRow("Cost model — flat dynamics", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);
        }

        // ------------------------------------------------------------------
        //  5. Cost smile section — scaled dynamics (strike-parameterised)
        //
        //     As above but uses a scaled (proportional) smile dynamics where
        //     the relative smile shape is preserved as spot moves.  Also
        //     calibrates exactly.
        // ------------------------------------------------------------------

        {
            FxCostSmileSectionScaledDynamics ss(expiryDate, spot, smileQuotes,
                                                forDiscount, domDiscount,
                                                deltaType, atmType,
                                                dc, settlement, Date(), true);
            printRow("Cost model — scaled dynamics", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);
        }

        // ------------------------------------------------------------------
        //  6. Cost smile section — flat dynamics with sticky delta
        //
        //     As the flat-dynamics model, but the smile moves with the
        //     forward (sticky delta).  The cost equation then involves the
        //     smile's slope and is solved as an ODE; the three cost ratios
        //     are fitted to the five quotes by least squares.
        // ------------------------------------------------------------------

        {
            FxCostSmileSectionStickyDelta ss(expiryDate, spot, smileQuotes, forDiscount,
                                                domDiscount, deltaType, atmType, dc, settlement,
                                                Date(), true);
            printRow("Cost model — sticky delta", ss, mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10,
                     mkt_bf10);
        }

        // ------------------------------------------------------------------
        //  7. Cost smile section — flat dynamics with convex sticky delta
        //
        //     As the flat-dynamics model, but the smile moves with the
        //     forward (sticky delta).  The cost equation then involves the
        //     smile's slope and is solved as an ODE; the three cost ratios
        //     are fitted to the five quotes by least squares.
        // ------------------------------------------------------------------

        {
            FxCostSmileSectionStickyDeltaConvex ss(expiryDate, spot, smileQuotes, forDiscount,
                                                   domDiscount, deltaType, atmType, dc, settlement,
                                                   Date(), true);
            printRow("Cost model — convex sticky delta", ss, mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10,
                     mkt_bf10);
        }

        // ------------------------------------------------------------------
        //  8. Quadratic smile section (delta-parameterised, 3 params)
        //
        //     Implied vol is a quadratic function of put delta:
        //       sigma(delta) = a*delta^2 + b*delta + c
        //     Three parameters => over-determined, best-fit via LM.
        // ------------------------------------------------------------------

        std::cout << "--- Delta-parameterised models ---\n\n";

        {
            FxQuadraticSmileSection ss(expiryDate, spot, smileQuotes,
                                       forDiscount, domDiscount,
                                       deltaType, atmType, dc, settlement);
            printRow("Quadratic in delta (3 params)", ss,
                     mkt_atm, mkt_rr25, mkt_bf25, mkt_rr10, mkt_bf10);

            std::cout << "    Calibrated params: a=" << std::setprecision(6) << ss.a()
                      << "  b=" << ss.b()
                      << "  c=" << ss.c() << "\n\n";
        }

        // ------------------------------------------------------------------
        //  Market reference row
        // ------------------------------------------------------------------

        std::cout << sep << "\n";
        Real mkt_v25c = mkt_atm + 0.5 * mkt_rr25 + mkt_bf25;
        Real mkt_v25p = mkt_atm - 0.5 * mkt_rr25 + mkt_bf25;
        Real mkt_v10c = mkt_atm + 0.5 * mkt_rr10 + mkt_bf10;
        Real mkt_v10p = mkt_atm - 0.5 * mkt_rr10 + mkt_bf10;
        std::cout << std::setw(w0) << std::left  << "Market (smile-strangle)"
                  << std::fixed    << std::setprecision(4)
                  << std::setw(w)  << std::right  << mkt_atm  * 100.0
                  << std::setw(w)  << std::right  << mkt_v25p * 100.0
                  << std::setw(w)  << std::right  << mkt_v25c * 100.0
                  << std::setw(w)  << std::right  << mkt_v10p * 100.0
                  << std::setw(w)  << std::right  << mkt_v10c * 100.0
                  << std::setw(w)  << std::right  << mkt_rr25 * 100.0
                  << std::setw(w)  << std::right  << mkt_bf25 * 100.0
                  << std::setw(w)  << std::right  << mkt_rr10 * 100.0
                  << std::setw(w)  << std::right  << mkt_bf10 * 100.0
                  << "\n";
        std::cout << sep << "\n\n";

        // ------------------------------------------------------------------
        //  Strike grid: show vol by strike for each model
        // ------------------------------------------------------------------

        std::cout << "--- Vol by strike (selected strikes) ---\n\n";

        // Use the cost flat dynamics model as reference for the ATM strike.
        FxCostSmileSectionFlatDynamics ref(expiryDate, spot, smileQuotes,
                                           forDiscount, domDiscount,
                                           deltaType, atmType,
                                           dc, settlement, Date(), true);
        Real fwd = ref.forward();
        std::vector<Real> strikes = { fwd * 0.85, fwd * 0.90, fwd * 0.95,
                                      fwd,
                                      fwd * 1.05, fwd * 1.10, fwd * 1.15 };

        // Header
        const int wk = 10;
        const int wv =  9;
        std::cout << std::setw(wk) << std::right << "Strike";
        for (Real k : strikes)
            std::cout << std::setw(wv) << std::right << std::fixed << std::setprecision(4) << k;
        std::cout << "\n" << std::string(wk + wv * strikes.size(), '-') << "\n";

        auto printStrikeRow = [&](const std::string& nm, FxSmileSection& ss) {
            std::cout << std::setw(wk) << std::left << nm;
            for (Real k : strikes)
                std::cout << std::setw(wv) << std::right << std::fixed
                          << std::setprecision(4) << ss.volByStrike(k) * 100.0;
            std::cout << "\n";
        };

        {
            FxPolynomialSmileSection s1(expiryDate, spot, smileQuotes,
                                        forDiscount, domDiscount, deltaType, atmType, dc, settlement);
            printStrikeRow("Polynomial", s1);
        }
        {
            FxSabrSmileSection s2(expiryDate, spot, smileQuotes,
                                  forDiscount, domDiscount, deltaType, atmType, dc, settlement);
            printStrikeRow("SABR", s2);
        }
        {
            FxSviSmileSection s3(expiryDate, spot, smileQuotes,
                                 forDiscount, domDiscount, deltaType, atmType, dc, settlement);
            printStrikeRow("SVI", s3);
        }
        {
            FxCostSmileSectionFlatDynamics s4(expiryDate, spot, smileQuotes,
                                              forDiscount, domDiscount, deltaType, atmType,
                                              dc, settlement, Date(), true);
            printStrikeRow("Cost-flat", s4);
        }
        {
            FxCostSmileSectionScaledDynamics s5(expiryDate, spot, smileQuotes,
                                                forDiscount, domDiscount, deltaType, atmType,
                                                dc, settlement, Date(), true);
            printStrikeRow("Cost-scaled", s5);
        }
        {
            FxQuadraticSmileSection s6(expiryDate, spot, smileQuotes,
                                       forDiscount, domDiscount, deltaType, atmType, dc, settlement);
            printStrikeRow("Quadratic-delta", s6);
        }
        std::cout << "\n";

        return 0;

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return 1;
    }
}
