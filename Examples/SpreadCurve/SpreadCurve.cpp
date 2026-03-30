/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*!
 Copyright (C) 2025 QuantLib contributors

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

/*!
    \example SpreadCurve.cpp

    Demonstrates the construction of a spread yield curve on top of the
    FOMC-anchored Fed Funds futures curve from the FedFundsFuturesCurve
    example, using PiecewiseSpreadYieldCurve<ZeroYield, ...>.

    Base curve:
      PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap> bootstrapped
      from 15 monthly 30-day Fed Funds futures contracts (Feb 2025 – Apr 2026),
      with 11 FOMC meeting dates injected as additional interpolation nodes and
      a forward-continuity penalty enforced at each futures pillar.

    Spread curves:
      Two PiecewiseSpreadYieldCurve<ZeroYield, ...> instances are bootstrapped
      from the same set of OIS swap helpers (rates ~10–70 bp above the futures
      curve), each storing and interpolating an additive zero-rate spread:

        1. Linear interpolation on zero-rate spreads
        2. Cubic (natural spline) interpolation on zero-rate spreads

    The example prints the bootstrapped spread nodes, a 6-month forward rate
    comparison between both spread curves and the base, and a repricing check.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/termstructures/yield/overnightindexfutureratehelper.hpp>
#include <ql/termstructures/yield/piecewisespreadyieldcurve.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace QuantLib;

int main() {
    try {
        std::cout << "\n=== Spread Yield Curve — Zero-Rate Spread Bootstrap ===\n\n";

        // -------------------------------------------------------------------
        // 1.  Evaluation date
        // -------------------------------------------------------------------
        const Date today(27, January, 2025);
        Settings::instance().evaluationDate() = today;

        const DayCounter dc = Actual360();
        const Calendar calendar = UnitedStates(UnitedStates::FederalReserve);
        auto ff = ext::make_shared<FedFunds>();

        std::cout << "Evaluation date : " << today << "\n\n";

        // -------------------------------------------------------------------
        // 2.  Base curve: FOMC-anchored Fed Funds futures
        //
        //     Identical construction to the FedFundsFuturesCurve example.
        //     15 monthly 30-day Fed Funds futures (Feb 2025 – Apr 2026) are
        //     bootstrapped with GlobalBootstrap, with:
        //       - 11 FOMC meeting dates as additional interpolation nodes
        //       - A forward-continuity penalty at each futures pillar so that
        //         the instantaneous forward is flat between FOMC meetings.
        // -------------------------------------------------------------------
        const std::vector<Date> fomcDates = {
            Date(19, March, 2025),
            Date(7, May, 2025),
            Date(18, June, 2025),
            Date(30, July, 2025),
            Date(17, September, 2025),  // -25 bp cut → 4.08 %
            Date(29, October, 2025),
            Date(10, December, 2025),   // -25 bp cut → 3.83 %
            Date(28, January, 2026),
            Date(18, March, 2026),
            Date(29, April, 2026),
            Date(10, June, 2026),
        };

        struct FuturesQuote { Month month; Year year; Real price; };

        const FuturesQuote futuresData[] = {
            {February,  2025, 95.670},
            {March,     2025, 95.670},
            {April,     2025, 95.670},
            {May,       2025, 95.670},
            {June,      2025, 95.670},
            {July,      2025, 95.670},
            {August,    2025, 95.670},
            {September, 2025, 95.787},  // cut Sep 17 → avg 4.213 %
            {October,   2025, 95.920},
            {November,  2025, 95.920},
            {December,  2025, 96.097},  // cut Dec 10 → avg 3.903 %
            {January,   2026, 96.170},
            {February,  2026, 96.170},
            {March,     2026, 96.170},
            {April,     2026, 96.170},
        };

        std::vector<ext::shared_ptr<RateHelper>> baseHelpers;
        baseHelpers.reserve(std::size(futuresData));

        for (const auto& fd : futuresData) {
            const Date valueDate(1, fd.month, fd.year);
            const Date maturityDate = Date::endOfMonth(valueDate) + 1;
            baseHelpers.push_back(ext::make_shared<OvernightIndexFutureRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(fd.price)),
                valueDate, maturityDate, ff,
                Handle<Quote>(),
                RateAveraging::Simple));
        }

        // Forward-continuity penalty at futures pillar nodes (see FedFundsFuturesCurve.cpp)
        std::vector<Time> futurePillarTimes;
        futurePillarTimes.reserve(baseHelpers.size());
        for (const auto& h : baseHelpers)
            futurePillarTimes.push_back(dc.yearFraction(today, h->pillarDate()));

        auto penaltyFn = [futurePillarTimes](const std::vector<Time>& times,
                                              const std::vector<Real>& data) -> Array {
            Array penalties(futurePillarTimes.size(), 0.0);
            for (Size k = 0; k < futurePillarTimes.size(); ++k) {
                auto it = std::lower_bound(times.begin(), times.end(), futurePillarTimes[k]);
                if (it == times.begin() || it == times.end())
                    continue;
                auto next_it = std::next(it);
                if (next_it == times.end())
                    continue;
                const Size i = std::distance(times.begin(), it);
                const Real fwd_left =
                    (std::log(data[i - 1]) - std::log(data[i])) / (times[i] - times[i - 1]);
                const Real fwd_right =
                    (std::log(data[i]) - std::log(data[i + 1])) / (times[i + 1] - times[i]);
                penalties[k] = fwd_left - fwd_right;
            }
            return penalties;
        };

        using FFFCurve = PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap>;

        auto additionalDatesFn = [&fomcDates]() { return fomcDates; };

        const FFFCurve::bootstrap_type bootstrap({}, additionalDatesFn, penaltyFn, 1.0e-12);

        auto baseCurve = ext::make_shared<FFFCurve>(today, baseHelpers, dc, LogLinear(), bootstrap);
        baseCurve->enableExtrapolation();

        Handle<YieldTermStructure> baseHandle(baseCurve);

        std::cout << "Base curve      : PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap>\n";
        std::cout << "                  15 Fed Funds futures + 11 FOMC nodes + continuity penalty\n\n";

        // -------------------------------------------------------------------
        // 3.  Spread instruments
        //
        //     OIS swaps at rates ~10–70 bp above the futures curve level,
        //     representing e.g. a credit or funding spread.  The spread curve
        //     bootstraps the additive zero-rate spread that reprices each swap
        //     against the futures base curve.
        //
        //     The futures curve extends to Apr 2026 (~15 months); beyond that
        //     it extrapolates flat forward.  The OIS helpers reaching 2Y–10Y
        //     will reprice against that extrapolated region.
        // -------------------------------------------------------------------
        struct OISQuote { Period tenor; Rate rate; };

        const OISQuote spreadData[] = {
            {1 * Months,  0.0443},   // ~+10 bp over 4.33 % base
            {3 * Months,  0.0452},   // ~+20 bp
            {6 * Months,  0.0465},   // ~+35 bp
            {1 * Years,   0.0475},   // ~+50 bp
            {2 * Years,   0.0480},   // ~+65 bp
            {3 * Years,   0.0475},   // ~+70 bp
            {5 * Years,   0.0460},   // ~+70 bp
            {7 * Years,   0.0450},   // ~+65 bp
            {10 * Years,  0.0440},   // ~+60 bp
        };

        std::vector<ext::shared_ptr<RateHelper>> spreadHelpers;
        for (const auto& q : spreadData)
            spreadHelpers.push_back(ext::make_shared<OISRateHelper>(
                2, q.tenor, q.rate, ff));

        // -------------------------------------------------------------------
        // 4.  Build spread curves with two different interpolations
        // -------------------------------------------------------------------
        using SpreadCurveLinear = PiecewiseSpreadYieldCurve<ZeroYield, Linear>;
        using SpreadCurveCubic  = PiecewiseSpreadYieldCurve<ZeroYield, Cubic>;

        auto spreadLinear = ext::make_shared<SpreadCurveLinear>(
            baseHandle, spreadHelpers, Linear());
        spreadLinear->enableExtrapolation();

        auto spreadCubic = ext::make_shared<SpreadCurveCubic>(
            baseHandle, spreadHelpers,
            Cubic(CubicInterpolation::Spline, true));
        spreadCubic->enableExtrapolation();

        std::cout << "Spread curves   : PiecewiseSpreadYieldCurve<ZeroYield, ...>\n";
        std::cout << "  (a) Linear interpolation on zero-rate spreads\n";
        std::cout << "  (b) Cubic  interpolation on zero-rate spreads\n\n";

        // -------------------------------------------------------------------
        // 5.  Report: bootstrapped spread nodes
        // -------------------------------------------------------------------
        std::cout << std::string(80, '=') << "\n";
        std::cout << "Bootstrapped zero-rate spread nodes\n";
        std::cout << std::string(80, '-') << "\n";
        std::cout << std::left
                  << std::setw(16) << "Date"
                  << std::setw(10) << "Time"
                  << std::setw(16) << "Spread (L) bp"
                  << std::setw(16) << "Spread (C) bp"
                  << std::setw(12) << "Base zero %"
                  << "Total zero (L) %\n";
        std::cout << std::string(80, '-') << "\n";

        const auto& datesL = spreadLinear->dates();
        const auto& dataL  = spreadLinear->data();
        const auto& dataC  = spreadCubic->data();

        for (Size i = 0; i < datesL.size(); ++i) {
            const Date& d = datesL[i];
            Time t = dc.yearFraction(today, d);
            Rate baseZero =
                (t > 0.0) ? baseCurve->zeroRate(d, dc, Continuous).rate() : 0.0;
            Rate totalZeroL =
                (t > 0.0) ? spreadLinear->zeroRate(d, dc, Continuous).rate() : 0.0;

            std::cout << std::left << std::setw(16) << d
                      << std::fixed << std::setprecision(4)
                      << std::setw(10) << t
                      << std::setw(16) << dataL[i] * 10000.0
                      << std::setw(16) << dataC[i] * 10000.0
                      << std::setw(12) << baseZero * 100.0
                      << totalZeroL * 100.0 << "\n";
        }

        // -------------------------------------------------------------------
        // 6.  Report: forward rate comparison
        //
        //     Semi-annual forward rates showing how interpolation choice
        //     affects the spread curve's forward profile.
        // -------------------------------------------------------------------
        std::cout << "\n" << std::string(74, '=') << "\n";
        std::cout << "6-month forward rates: base vs. spread (Linear / Cubic)\n";
        std::cout << std::string(74, '-') << "\n";
        std::cout << std::left
                  << std::setw(16) << "Start"
                  << std::setw(16) << "End"
                  << std::setw(14) << "Base fwd %"
                  << std::setw(14) << "Linear fwd %"
                  << "Cubic fwd %\n";
        std::cout << std::string(74, '-') << "\n";

        for (int m = 0; m <= 108; m += 6) {
            Date d1 = calendar.advance(today, m * Months);
            Date d2 = calendar.advance(today, (m + 6) * Months);

            Rate fwdBase = baseCurve->forwardRate(d1, d2, dc, Continuous).rate();
            Rate fwdL    = spreadLinear->forwardRate(d1, d2, dc, Continuous).rate();
            Rate fwdC    = spreadCubic->forwardRate(d1, d2, dc, Continuous).rate();

            std::cout << std::left << std::setw(16) << d1 << std::setw(16) << d2
                      << std::fixed << std::setprecision(4)
                      << std::setw(14) << fwdBase * 100.0
                      << std::setw(14) << fwdL * 100.0
                      << fwdC * 100.0 << "\n";
        }

        // -------------------------------------------------------------------
        // 7.  Report: helper repricing check (Linear spread curve)
        // -------------------------------------------------------------------
        std::cout << "\n" << std::string(70, '=') << "\n";
        std::cout << "Helper repricing — Linear spread curve\n";
        std::cout << std::string(70, '-') << "\n";
        std::cout << std::left
                  << std::setw(16) << "Value date"
                  << std::setw(16) << "Maturity"
                  << std::setw(12) << "Market"
                  << std::setw(12) << "Implied"
                  << "Error (bp)\n";
        std::cout << std::string(70, '-') << "\n";

        Real maxErr = 0.0;
        for (const auto& h : spreadHelpers) {
            Real market  = h->quote()->value();
            Real implied = h->impliedQuote();
            Real errBp   = (implied - market) * 10000.0;
            maxErr = std::max(maxErr, std::abs(errBp));

            std::cout << std::left << std::setw(16) << h->earliestDate()
                      << std::setw(16) << h->maturityDate()
                      << std::fixed << std::setprecision(6)
                      << std::setw(12) << market
                      << std::setw(12) << implied
                      << std::setprecision(4) << errBp << "\n";
        }

        std::cout << "\nMax repricing error: " << std::scientific
                  << std::setprecision(2) << maxErr << " bp\n\n";

        // -------------------------------------------------------------------
        // 8.  Write CSV for external plotting
        //
        //     Dense monthly grid (1M step, 10Y horizon).  Columns:
        //       date (YYYY-MM-DD), time, base_zero, linear_zero, cubic_zero,
        //       linear_spread_bp, cubic_spread_bp
        //     Also writes the FOMC dates and spread pillar dates so that
        //     the plotting script can mark them.
        // -------------------------------------------------------------------
        {
            // Helper: format a QuantLib Date as YYYY-MM-DD
            auto isoDate = [](const Date& d) {
                std::ostringstream s;
                s << d.year()
                  << "-" << std::setfill('0') << std::setw(2) << int(d.month())
                  << "-" << std::setfill('0') << std::setw(2) << d.dayOfMonth();
                return s.str();
            };

            std::ofstream csv("spread_curves.csv");
            csv << std::fixed << std::setprecision(8);
            csv << "date,time,base_zero,linear_zero,cubic_zero,"
                   "linear_spread_bp,cubic_spread_bp\n";

            for (int m = 1; m <= 120; ++m) {
                Date d = calendar.advance(today, m * Months);
                Time t = dc.yearFraction(today, d);
                Rate bz = baseCurve->zeroRate(d, dc, Continuous).rate();
                Rate lz = spreadLinear->zeroRate(d, dc, Continuous).rate();
                Rate cz = spreadCubic->zeroRate(d, dc, Continuous).rate();
                csv << isoDate(d) << "," << t
                    << "," << bz * 100.0
                    << "," << lz * 100.0
                    << "," << cz * 100.0
                    << "," << (lz - bz) * 10000.0
                    << "," << (cz - bz) * 10000.0
                    << "\n";
            }

            // Spread pillar nodes
            std::ofstream pillars("spread_pillars.csv");
            pillars << std::fixed << std::setprecision(8);
            pillars << "date,time,linear_spread_bp,cubic_spread_bp,"
                       "base_zero,linear_zero\n";
            for (Size i = 1; i < datesL.size(); ++i) {
                const Date& d = datesL[i];
                Time t = dc.yearFraction(today, d);
                Rate bz = baseCurve->zeroRate(d, dc, Continuous).rate();
                Rate lz = spreadLinear->zeroRate(d, dc, Continuous).rate();
                pillars << isoDate(d) << "," << t
                        << "," << dataL[i] * 10000.0
                        << "," << dataC[i] * 10000.0
                        << "," << bz * 100.0
                        << "," << lz * 100.0
                        << "\n";
            }

            // FOMC dates
            std::ofstream fomc("fomc_dates.csv");
            fomc << "date,time\n";
            for (const auto& fd : fomcDates) {
                Time t = dc.yearFraction(today, fd);
                if (t <= 10.0)
                    fomc << isoDate(fd) << "," << t << "\n";
            }

            std::cout << "CSV files written: spread_curves.csv, "
                         "spread_pillars.csv, fomc_dates.csv\n\n";
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\nUnknown error\n";
        return 1;
    }
}
