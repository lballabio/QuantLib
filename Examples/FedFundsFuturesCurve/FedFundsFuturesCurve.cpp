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
    \example FedFundsFuturesCurve.cpp

    Bootstraps a Fed Funds discount curve from 15 monthly 30-day Fed Funds
    futures using QuantLib's GlobalBootstrap, with the next 11 FOMC meeting
    dates (after the first futures start date) injected as additional
    interpolation nodes.

    The Federal Reserve changes the policy rate only at FOMC meetings, so the
    instantaneous forward rate should be a step function that can jump only on
    those dates.  By adding FOMC dates as additional interpolation knots and
    applying a forward-rate continuity penalty at each futures pillar (month-
    end) node, the GlobalBootstrap finds a curve whose forwards are flat
    between consecutive FOMC meetings.

    Penalty design:
      At every futures pillar (1st of each month) we penalise the difference
      between the left and right instantaneous forward:

          fwd(t_{i-1}, t_i)  =  fwd(t_i, t_{i+1})

      With log-linear interpolation on discount factors this is equivalent to
      penalising the second difference in log-discount space.  FOMC date nodes
      carry no penalty, so the forward is free to jump there.

    System dimensions:
      Grid nodes (non-reference):  15 futures pillars + 11 FOMC dates = 26
      Equations:                   15 helpers (futures)
                                 + 15 continuity penalties             = 30
      30 >= 26 - 1 = 25  ✓  (slightly overdetermined, solved via LM)

    Scenario (illustrative):
      Evaluation date:  27 January 2025
      Current FF rate:  4.33 %
      -25 bp at Sep 17 2025  →  4.08 %
      -25 bp at Dec 10 2025  →  3.83 %
      On hold through Jun 2026

    Interpolation: log-linear on discount factors.
*/

#include <ql/qldefines.hpp>
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#    include <ql/auto_link.hpp>
#endif

#include <ql/cashflows/rateaveraging.hpp>
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/globalbootstrap.hpp>
#include <ql/termstructures/yield/overnightindexfutureratehelper.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace QuantLib;

int main() {
    try {
        std::cout << "\n=== Fed Funds Futures Curve — FOMC-Anchored Global Bootstrap ===\n\n";

        // -----------------------------------------------------------------------
        // 1.  Evaluation date
        // -----------------------------------------------------------------------
        const Date today(27, January, 2025);
        Settings::instance().evaluationDate() = today;

        std::cout << "Evaluation date : " << today << "\n";
        std::cout << "Interpolation   : log-linear on discount factors\n";
        std::cout << "Bootstrap       : GlobalBootstrap (Levenberg-Marquardt)\n\n";

        // -----------------------------------------------------------------------
        // 2.  FOMC decision dates used as additional interpolation nodes
        //
        //     Only dates on or after the first futures value date (Feb 1) are
        //     included.  The Jan 29 meeting is omitted because it falls before
        //     any futures contract starts, leaving its node unconstrained and
        //     causing a near-degenerate period of just two days at the short end.
        // -----------------------------------------------------------------------
        const std::vector<Date> fomcDates = {
            Date(19, March, 2025),     //  hold  (4.33 %)
            Date(7, May, 2025),        //  hold
            Date(18, June, 2025),      //  hold
            Date(30, July, 2025),      //  hold
            Date(17, September, 2025), //  CUT  → 4.08 %
            Date(29, October, 2025),   //  hold
            Date(10, December, 2025),  //  CUT  → 3.83 %
            Date(28, January, 2026),   //  hold
            Date(18, March, 2026),     //  hold
            Date(29, April, 2026),     //  hold
            Date(10, June, 2026),      //  hold
        };

        // -----------------------------------------------------------------------
        // 3.  30-day Fed Funds futures (Feb 2025 – Apr 2026, 15 contracts)
        //
        //     Price = 100 – (arithmetic average daily FF rate over the month)
        //
        //     Transition months:
        //       Sep 2025:  16 d × 4.33 % + 14 d × 4.08 %  →  avg 4.213 %
        //       Dec 2025:   9 d × 4.08 % + 22 d × 3.83 %  →  avg 3.903 %
        // -----------------------------------------------------------------------
        struct FuturesQuote {
            Month month;
            Year year;
            Real price;
        };

        const FuturesQuote futuresData[] = {
            {February, 2025, 95.670},  // 4.330 % all month
            {March, 2025, 95.670},     // 4.330 % all month
            {April, 2025, 95.670},     // 4.330 % all month
            {May, 2025, 95.670},       // 4.330 % all month
            {June, 2025, 95.670},      // 4.330 % all month
            {July, 2025, 95.670},      // 4.330 % all month
            {August, 2025, 95.670},    // 4.330 % all month
            {September, 2025, 95.787}, // cut Sep 17 → avg 4.213 %
            {October, 2025, 95.920},   // 4.080 % all month
            {November, 2025, 95.920},  // 4.080 % all month
            {December, 2025, 96.097},  // cut Dec 10 → avg 3.903 %
            {January, 2026, 96.170},   // 3.830 % all month
            {February, 2026, 96.170},  // 3.830 % all month
            {March, 2026, 96.170},     // 3.830 % all month
            {April, 2026, 96.170},     // 3.830 % all month
        };

        // -----------------------------------------------------------------------
        // 4.  Build rate helpers
        //     Value date  = 1st calendar day of the contract month
        //     Maturity    = end-of-month + 1 = 1st calendar day of next month
        //     Averaging   = Simple (arithmetic average), per 30-day FF spec
        // -----------------------------------------------------------------------
        const DayCounter dc = Actual360();
        auto ff = ext::make_shared<FedFunds>();

        std::vector<ext::shared_ptr<RateHelper>> helpers;
        helpers.reserve(std::size(futuresData));

        for (const auto& fd : futuresData) {
            const Date valueDate(1, fd.month, fd.year);
            const Date maturityDate = Date::endOfMonth(valueDate) + 1;

            helpers.push_back(ext::make_shared<OvernightIndexFutureRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(fd.price)), valueDate, maturityDate, ff,
                Handle<Quote>(),         // no convexity adjustment
                RateAveraging::Simple)); // arithmetic average (30-day FF convention)
        }

        // -----------------------------------------------------------------------
        // 5.  Penalty function: forward-rate continuity at futures pillar nodes
        //
        //     The FOMC dates are the economically meaningful kink points in the
        //     forward curve.  Futures pillars (end of each month) are merely
        //     technical bootstrap nodes with no associated rate decision.
        //     Enforcing forward-rate continuity there prevents spurious kinks.
        //
        //     With log-linear interpolation on discounts the instantaneous
        //     forward between consecutive grid nodes i-1 and i is:
        //
        //         fwd(i-1, i) = -(log D[i] - log D[i-1]) / (t[i] - t[i-1])
        //
        //     The continuity penalty at node i is:
        //
        //         fwd(i-1, i) - fwd(i, i+1) = 0
        // -----------------------------------------------------------------------

        // Capture futures pillar times after helpers are constructed.
        std::vector<Time> futurePillarTimes;
        futurePillarTimes.reserve(helpers.size());
        for (const auto& h : helpers)
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

        // -----------------------------------------------------------------------
        // 6.  Construct the curve
        // -----------------------------------------------------------------------
        using Curve = PiecewiseYieldCurve<Discount, LogLinear, GlobalBootstrap>;

        auto additionalDatesFn = [&fomcDates]() { return fomcDates; };

        const Curve::bootstrap_type bootstrap({}, // no additional helpers
                                              additionalDatesFn, penaltyFn,
                                              1.0e-12); // accuracy

        const auto curve = ext::make_shared<Curve>(today, helpers, dc, LogLinear(), bootstrap);
        curve->enableExtrapolation();

        // -----------------------------------------------------------------------
        // 7.  Report: full interpolation grid
        // -----------------------------------------------------------------------
        // Build a lookup set to label FOMC vs futures pillar nodes.
        const std::vector<Date> fomcSet(fomcDates.begin(), fomcDates.end());

        std::cout << std::string(74, '=') << "\n";
        std::cout << "Interpolation grid\n";
        std::cout << std::string(74, '-') << "\n";
        std::cout << std::left << std::setw(22) << "Date" << std::setw(12) << "Node type"
                  << std::setw(10) << "Time" << std::setw(18) << "Discount factor"
                  << "Zero rate (%)\n";
        std::cout << std::string(74, '-') << "\n";

        for (const Date& d : curve->dates()) {
            if (d == today)
                continue;
            const bool isFomc = std::binary_search(fomcSet.begin(), fomcSet.end(), d);
            const Time t = dc.yearFraction(today, d);
            const Real df = curve->discount(d);
            const Real z = curve->zeroRate(d, dc, Continuous).rate();

            std::cout << std::left << std::setw(22) << d << std::setw(12)
                      << (isFomc ? "FOMC" : "futures") << std::fixed << std::setprecision(4)
                      << std::setw(10) << t << std::setw(18) << df << z * 100.0 << "\n";
        }

        // -----------------------------------------------------------------------
        // 8.  Report: implied overnight forward rate between FOMC meetings
        //     With FOMC-date nodes and forward continuity at futures pillars,
        //     the forward should be flat between meetings and step at each one.
        // -----------------------------------------------------------------------
        std::cout << "\n" << std::string(58, '=') << "\n";
        std::cout << "Implied overnight forward rate between FOMC meetings\n";
        std::cout << std::string(58, '-') << "\n";
        std::cout << std::left << std::setw(22) << "From" << std::setw(22) << "To"
                  << "Fwd rate (%)\n";
        std::cout << std::string(58, '-') << "\n";

        auto printForward = [&](const Date& d1, const Date& d2) {
            const Real df1 = curve->discount(d1);
            const Real df2 = curve->discount(d2);
            const Time dt = dc.yearFraction(d1, d2);
            const Real fwd = (dt > 1.0e-10) ? (df1 / df2 - 1.0) / dt : 0.0;
            std::cout << std::left << std::setw(22) << d1 << std::setw(22) << d2 << std::fixed
                      << std::setprecision(4) << fwd * 100.0 << " %\n";
        };

        // From today to first FOMC
        printForward(today, fomcDates.front());
        for (Size i = 0; i + 1 < fomcDates.size(); ++i)
            printForward(fomcDates[i], fomcDates[i + 1]);

        // -----------------------------------------------------------------------
        // 9.  Report: helper repricing check
        // -----------------------------------------------------------------------
        std::cout << "\n" << std::string(70, '=') << "\n";
        std::cout << "Helper repricing  (market price vs. curve-implied price)\n";
        std::cout << std::string(70, '-') << "\n";
        std::cout << std::left << std::setw(16) << "Value date" << std::setw(16) << "Maturity"
                  << std::setw(12) << "Market" << std::setw(12) << "Implied"
                  << "Error (bp)\n";
        std::cout << std::string(70, '-') << "\n";

        Real maxErr = 0.0;
        for (const auto& h : helpers) {
            const Real market = h->quote()->value();
            const Real implied = h->impliedQuote();
            const Real errBp = (market - implied) * 10000.0;
            maxErr = std::max(maxErr, std::abs(market - implied));

            std::cout << std::left << std::setw(16) << h->earliestDate() << std::setw(16)
                      << h->maturityDate() << std::fixed << std::setprecision(6) << std::setw(12)
                      << market << std::setw(12) << implied << std::setprecision(4) << errBp
                      << "\n";
        }

        std::cout << "\nMax repricing error: " << std::scientific << std::setprecision(2)
                  << maxErr * 10000.0 << " bp\n\n";

        // -----------------------------------------------------------------------
        // 10. Export daily overnight forward rates to CSV for plotting
        //     For each calendar day d in [today, today + 13 months]:
        //       rate = simple forward from d to d+1 (1 calendar day)
        //     This directly mirrors how the Fed Funds rate accrues daily.
        // -----------------------------------------------------------------------
        const std::string csvPath = "fedfunds_forwards.csv";
        std::ofstream csv(csvPath);
        csv << "date,forward_rate_pct\n";

        const Date endDate = today + Period(13, Months);
        for (Date d = today; d < endDate; d++) {
            const Date d2 = d + 1;
            const Real fwd = curve->forwardRate(d, d2, dc, Simple).rate();
            // ISO date string: YYYY-MM-DD
            csv << d.year() << "-" << std::setw(2) << std::setfill('0') << int(d.month()) << "-"
                << std::setw(2) << std::setfill('0') << d.dayOfMonth() << "," << std::fixed
                << std::setprecision(6) << fwd * 100.0 << "\n";
        }
        csv.close();
        std::cout << "Daily forward rates written to: " << csvPath << "\n\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\nUnknown error\n";
        return 1;
    }
}
