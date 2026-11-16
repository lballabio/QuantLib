/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

#include "fxtestmarkets.hpp"
#include <ql/indexes/ibor/fedfunds.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/settings.hpp>
#include <ql/termstructures/yield/oisratehelper.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/calendars/jointcalendar.hpp>
#include <ql/time/calendars/southafrica.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    namespace {

        void requireEvaluationDate(const Date& asOf) {
            QL_REQUIRE(Settings::instance().evaluationDate() == asOf,
                       "the evaluation date (" << Settings::instance().evaluationDate()
                                               << ") must be the as-of date (" << asOf << ")");
        }

        ext::shared_ptr<YieldTermStructure>
        logLinearCurve(const Date& asOf, std::vector<ext::shared_ptr<RateHelper>> helpers) {
            auto curve = ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear>>(
                asOf, std::move(helpers), Actual365Fixed());
            curve->enableExtrapolation();
            // bootstrap now, so that bad quotes fail here rather than at first use
            curve->discount(curve->maxDate());
            return curve;
        }

        // the vol quotes, in vol points as quoted, for the tenors below
        struct VolRow {
            Real atm, rr25, bf25, rr10, bf10;
        };

        const std::vector<Period> volTenors = {1 * Days,   1 * Weeks,  2 * Weeks, 1 * Months,
                                               2 * Months, 3 * Months, 6 * Months, 9 * Months,
                                               1 * Years,  18 * Months, 2 * Years};

        FxVolQuotes volQuotes(const std::vector<VolRow>& rows) {
            QL_REQUIRE(rows.size() == volTenors.size(), "one row of vols per tenor needed");
            FxVolQuotes quotes;
            quotes.tenors = volTenors;
            quotes.deltas = {0.25, 0.10};
            for (const VolRow& r : rows) {
                quotes.atms.push_back(r.atm / 100.0);
                quotes.riskReversals.push_back({r.rr25 / 100.0, r.rr10 / 100.0});
                quotes.butterflies.push_back({r.bf25 / 100.0, r.bf10 / 100.0});
            }
            return quotes;
        }

        // points in pips, i.e. 1e-4 of the spot rate's units
        FxForwardQuotes forwardQuotes(Real spot,
                                      Real on,
                                      Real tn,
                                      Real sn,
                                      const std::vector<Period>& tenors,
                                      const std::vector<Real>& pips) {
            FxForwardQuotes quotes{spot, on * 1.0e-4, tn * 1.0e-4, sn * 1.0e-4, tenors, {}};
            for (Real p : pips)
                quotes.points.push_back(p * 1.0e-4);
            return quotes;
        }

        // rates in percent
        OisQuotes oisQuotes(ext::shared_ptr<OvernightIndex> index,
                            Rate fixing,
                            const std::vector<Period>& tenors,
                            const std::vector<Rate>& percents) {
            OisQuotes quotes{std::move(index), fixing / 100.0, tenors, {}};
            for (Rate r : percents)
                quotes.rates.push_back(r / 100.0);
            return quotes;
        }

        FxTestMarket build(const Date& asOf,
                           const FxPairConventions& conventions,
                           FxForwardQuotes forwards,
                           OisQuotes ois,
                           const FxVolQuotes& vols,
                           const FxVolSurfaceConfig& config) {
            Settings::instance().evaluationDate() = asOf;
            const Handle<YieldTermStructure> usd(bootstrapOisCurve(asOf, ois));
            const bool usdIsBase = conventions.foreignCurrency() == "USD";
            const Handle<YieldTermStructure> other(bootstrapFxImpliedCurve(
                asOf, conventions.settlement(), forwards, usdIsBase, usd));
            FxPairMarket market(asOf, conventions, forwards.spot, usdIsBase ? usd : other,
                                usdIsBase ? other : usd, vols, {}, config);
            return {std::move(market), std::move(forwards), std::move(ois)};
        }

    }

    ext::shared_ptr<YieldTermStructure> bootstrapOisCurve(const Date& asOf,
                                                          const OisQuotes& quotes) {
        requireEvaluationDate(asOf);
        QL_REQUIRE(quotes.index, "no OIS index given");
        QL_REQUIRE(quotes.tenors.size() == quotes.rates.size() && !quotes.tenors.empty(),
                   quotes.rates.size() << " OIS rates given for " << quotes.tenors.size()
                                       << " tenors");
        const Calendar calendar = quotes.index->fixingCalendar();
        std::vector<ext::shared_ptr<RateHelper>> helpers;
        helpers.push_back(ext::make_shared<DepositRateHelper>(
            makeQuoteHandle(quotes.fixing), 1 * Days, 0, calendar, Following, false,
            quotes.index->dayCounter()));
        for (Size i = 0; i < quotes.tenors.size(); ++i)
            helpers.push_back(ext::make_shared<OISRateHelper>(
                2, quotes.tenors[i], makeQuoteHandle(quotes.rates[i]), quotes.index,
                Handle<YieldTermStructure>(), true, 2, ModifiedFollowing, Annual, calendar));
        return logLinearCurve(asOf, std::move(helpers));
    }

    ext::shared_ptr<YieldTermStructure>
    bootstrapFxImpliedCurve(const Date& asOf,
                            const FxSettleConvention& settlement,
                            const FxForwardQuotes& quotes,
                            bool collateralIsBase,
                            const Handle<YieldTermStructure>& collateralCurve) {
        requireEvaluationDate(asOf);
        QL_REQUIRE(quotes.tenors.size() == quotes.points.size() && !quotes.tenors.empty(),
                   quotes.points.size() << " forward points given for " << quotes.tenors.size()
                                        << " tenors");
        QL_REQUIRE(settlement.spotLag() == 2, "ON and TN need a spot lag of 2 days");
        const Calendar& calendar = settlement.calendar();
        const Date today = calendar.adjust(asOf);
        const Date tomorrow = calendar.advance(today, 1, Days);
        const Date spotDate = settlement.valueDate(asOf);
        QL_REQUIRE(calendar.advance(tomorrow, 1, Days) == spotDate,
                   "spot is not the business day after tomorrow");

        // the outrights at today, tomorrow and spot, chained from the points
        const Real atTomorrow = quotes.spot - quotes.tn;
        const Real atToday = atTomorrow - quotes.on;
        auto helper = [&](Real points, Real start, const Date& from, const Date& to) {
            return ext::make_shared<FxSwapRateHelper>(makeQuoteHandle(points),
                                                      makeQuoteHandle(start), from, to,
                                                      collateralIsBase, collateralCurve);
        };
        std::vector<ext::shared_ptr<RateHelper>> helpers;
        helpers.push_back(helper(quotes.on, atToday, today, tomorrow));
        helpers.push_back(helper(quotes.tn, atTomorrow, tomorrow, spotDate));
        helpers.push_back(
            helper(quotes.sn, quotes.spot, spotDate, calendar.advance(spotDate, 1, Days)));
        for (Size i = 0; i < quotes.tenors.size(); ++i)
            helpers.push_back(helper(
                quotes.points[i], quotes.spot, spotDate,
                calendar.advance(spotDate, quotes.tenors[i], ModifiedFollowing, true)));
        return logLinearCurve(asOf, std::move(helpers));
    }

    FxTestMarket eurusdJan2024(const FxVolSurfaceConfig& config) {
        const Date asOf(2, January, 2024);
        const FxPairConventions conventions(
            "EUR", "USD",
            FxSettleConvention(
                JointCalendar(TARGET(), UnitedStates(UnitedStates::FederalReserve)), 2),
            FxExpiryCut(10), DeltaVolQuote::Spot, DeltaVolQuote::AtmDeltaNeutral,
            FxRrBfQuotes::SmileStrangle);
        const FxVolQuotes vols = volQuotes({
            // ATM    RR25    BF25   RR10    BF10
            {6.850, -0.125, 0.125, -0.250, 0.450},  // 1D
            {7.625, -0.075, 0.075, -0.150, 0.288},  // 1W
            {7.450, -0.100, 0.075, -0.225, 0.263},  // 2W
            {7.300, -0.287, 0.075, -0.400, 0.275},  // 1M
            {6.875, -0.300, 0.088, -0.625, 0.325},  // 2M
            {6.775, -0.400, 0.113, -0.800, 0.413},  // 3M
            {6.675, -0.625, 0.163, -1.250, 0.575},  // 6M
            {6.625, -0.775, 0.213, -1.525, 0.737},  // 9M
            {6.775, -0.875, 0.213, -1.700, 0.800},  // 1Y
            {6.984, -0.810, 0.230, -1.580, 0.760},  // 18M
            {7.025, -0.825, 0.238, -1.600, 0.875},  // 2Y
        });
        FxForwardQuotes forwards = forwardQuotes(
            1.0950, 0.439, 0.43, 0.423,
            {1 * Weeks, 2 * Weeks, 1 * Months, 2 * Months, 3 * Months, 6 * Months, 9 * Months,
             1 * Years, 15 * Months, 18 * Months, 2 * Years},
            {2.97, 5.97, 13.9, 26.25, 40.62, 83.19, 128.2, 180.03, 226.97, 274.32, 364.04});
        OisQuotes ois = oisQuotes(
            ext::make_shared<Sofr>(), 5.40,
            {1 * Weeks, 2 * Weeks, 1 * Months, 2 * Months, 3 * Months, 4 * Months, 5 * Months,
             6 * Months, 7 * Months, 8 * Months, 9 * Months, 10 * Months, 11 * Months,
             12 * Months, 18 * Months, 2 * Years, 3 * Years},
            {5.33845, 5.3395, 5.3449, 5.3465, 5.33, 5.2934, 5.2362, 5.1702, 5.1115, 5.0475,
             4.9875, 4.9268, 4.8655, 4.8003, 4.4045, 4.141, 3.8292});
        return build(asOf, conventions, std::move(forwards), std::move(ois), vols, config);
    }

    FxTestMarket usdzarMar2020(const FxVolSurfaceConfig& config) {
        const Date asOf(25, March, 2020);
        const FxPairConventions conventions(
            "USD", "ZAR",
            FxSettleConvention(
                JointCalendar(SouthAfrica(), UnitedStates(UnitedStates::FederalReserve)), 2),
            FxExpiryCut(10), DeltaVolQuote::PaSpot, DeltaVolQuote::AtmDeltaNeutral,
            FxRrBfQuotes::SmileStrangle);
        const FxVolQuotes vols = volQuotes({
            // ATM    RR25   BF25   RR10   BF10
            {25.820, 2.377, 0.647, 4.235, 2.000},  // 1D
            {25.275, 3.373, 0.955, 5.945, 3.130},  // 1W
            {23.070, 3.670, 1.000, 6.450, 3.145},  // 2W
            {20.605, 3.955, 1.090, 7.250, 3.525},  // 1M
            {19.045, 4.100, 1.090, 7.612, 3.467},  // 2M
            {18.480, 4.100, 1.090, 7.785, 3.455},  // 3M
            {17.390, 4.135, 1.095, 8.005, 3.430},  // 6M
            {16.490, 4.170, 1.102, 8.080, 3.420},  // 9M
            {15.975, 4.148, 1.110, 8.030, 3.410},  // 1Y
            {16.155, 4.232, 1.130, 8.253, 3.370},  // 18M
            {16.230, 4.428, 1.167, 8.785, 3.575},  // 2Y
        });
        FxForwardQuotes forwards = forwardQuotes(
            17.3345, 27.5, 27.0, 80.0,
            {1 * Weeks, 2 * Weeks, 1 * Months, 2 * Months, 3 * Months, 6 * Months, 9 * Months,
             1 * Years, 15 * Months, 18 * Months, 2 * Years},
            {191, 468, 813, 1467, 2105, 3870, 5565, 7275, 9410, 11420, 15590});
        OisQuotes ois = oisQuotes(
            ext::make_shared<FedFunds>(), 0.10,
            {1 * Weeks, 2 * Weeks, 1 * Months, 2 * Months, 3 * Months, 4 * Months, 5 * Months,
             6 * Months, 9 * Months, 12 * Months, 18 * Months, 2 * Years, 3 * Years},
            {0.066, 0.084, 0.0935, 0.091665, 0.087045, 0.083, 0.078, 0.075, 0.071, 0.074, 0.098,
             0.12508, 0.194});
        return build(asOf, conventions, std::move(forwards), std::move(ois), vols, config);
    }

}
