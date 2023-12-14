/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/termstructures/basisswapratehelpers.hpp>
#include <ql/indexes/ibor/sofr.hpp>
#include <ql/indexes/ibor/usdlibor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/time/calendars/unitedstates.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BasisSwapRateHelpersTests)

namespace {

    struct BasisSwapQuote {
        Integer n;
        TimeUnit units;
        Spread basis;
    };

    void testIborIborBootstrap(bool bootstrapBaseCurve) {
        std::vector<BasisSwapQuote> quotes = {
            { 1, Years,  0.0010 },
            { 2, Years,  0.0012 },
            { 3, Years,  0.0015 },
            { 5, Years,  0.0015 },
            { 8, Years,  0.0018 },
            { 10, Years, 0.0020 },
            { 15, Years, 0.0021 },
            { 20, Years, 0.0021 },
        };

        auto settlementDays = 2;
        auto calendar = UnitedStates(UnitedStates::GovernmentBond);
        auto convention = Following;
        auto endOfMonth = false;

        Handle<YieldTermStructure> knownForecastCurve(flatRate(0.01, Actual365Fixed()));
        Handle<YieldTermStructure> discountCurve(flatRate(0.005, Actual365Fixed()));

        ext::shared_ptr<IborIndex> baseIndex, otherIndex;

        if (bootstrapBaseCurve) {
            baseIndex = ext::make_shared<USDLibor>(3 * Months);
            otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
        } else {
            baseIndex = ext::make_shared<USDLibor>(3 * Months, knownForecastCurve);
            otherIndex = ext::make_shared<USDLibor>(6 * Months);
        }

        std::vector<ext::shared_ptr<RateHelper>> helpers;
        for (auto q : quotes) {
            auto h = ext::make_shared<IborIborBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis)),
                Period(q.n, q.units), settlementDays, calendar, convention, endOfMonth,
                baseIndex, otherIndex, discountCurve, bootstrapBaseCurve);
            helpers.push_back(h);
        }

        auto bootstrappedCurve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>
            (0, calendar, helpers, Actual365Fixed());

        Date today = Settings::instance().evaluationDate();
        Date spot = calendar.advance(today, settlementDays, Days);

        if (bootstrapBaseCurve) {
            baseIndex = ext::make_shared<USDLibor>(3 * Months, Handle<YieldTermStructure>(bootstrappedCurve));
            otherIndex = ext::make_shared<USDLibor>(6 * Months, knownForecastCurve);
        } else {
            baseIndex = ext::make_shared<USDLibor>(3 * Months, knownForecastCurve);
            otherIndex = ext::make_shared<USDLibor>(6 * Months, Handle<YieldTermStructure>(bootstrappedCurve));
        }

        for (auto q : quotes) {
            // create swaps and check they're fair
            Date maturity = calendar.advance(spot, q.n, q.units, convention);

            Schedule s1 =
                MakeSchedule()
                .from(spot).to(maturity)
                .withTenor(baseIndex->tenor())
                .withCalendar(calendar)
                .withConvention(convention)
                .withRule(DateGeneration::Forward);
            Leg leg1 = IborLeg(s1, baseIndex)
                .withSpreads(q.basis)
                .withNotionals(100.0);

            Schedule s2 =
                MakeSchedule()
                .from(spot).to(maturity)
                .withTenor(otherIndex->tenor())
                .withCalendar(calendar)
                .withConvention(convention)
                .withRule(DateGeneration::Forward);
            Leg leg2 = IborLeg(s2, otherIndex)
                .withNotionals(100.0);

            Swap swap(leg1, leg2);
            swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));

            Real NPV = swap.NPV();
            Real tolerance = 1e-8;
            if (std::fabs(NPV) > tolerance) {
                BOOST_ERROR("Failed to price fair " << q.n << "-year(s) swap:"
                            << "\n    calculated: " << NPV);
            }
        }
    }

    void testOvernightIborBootstrap(bool externalDiscountCurve) {
        std::vector<BasisSwapQuote> quotes = {
            { 1, Years,  0.0010 },
            { 2, Years,  0.0012 },
            { 3, Years,  0.0015 },
            { 5, Years,  0.0015 },
            { 8, Years,  0.0018 },
            { 10, Years, 0.0020 },
            { 15, Years, 0.0021 },
            { 20, Years, 0.0021 },
        };

        auto settlementDays = 2;
        auto calendar = UnitedStates(UnitedStates::GovernmentBond);
        auto convention = Following;
        auto endOfMonth = false;

        Handle<YieldTermStructure> knownForecastCurve(flatRate(0.01, Actual365Fixed()));

        RelinkableHandle<YieldTermStructure> discountCurve;
        if (externalDiscountCurve)
            discountCurve.linkTo(flatRate(0.005, Actual365Fixed()));

        auto baseIndex = ext::make_shared<Sofr>(knownForecastCurve);
        auto otherIndex = ext::make_shared<USDLibor>(6 * Months);

        std::vector<ext::shared_ptr<RateHelper>> helpers;
        for (auto q : quotes) {
            auto h = ext::make_shared<OvernightIborBasisSwapRateHelper>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(q.basis)),
                Period(q.n, q.units), settlementDays, calendar, convention, endOfMonth,
                baseIndex, otherIndex, discountCurve);
            helpers.push_back(h);
        }

        auto bootstrappedCurve = ext::make_shared<PiecewiseYieldCurve<ZeroYield, Linear>>
            (0, calendar, helpers, Actual365Fixed());

        Date today = Settings::instance().evaluationDate();
        Date spot = calendar.advance(today, settlementDays, Days);

        otherIndex = ext::make_shared<USDLibor>(6 * Months, Handle<YieldTermStructure>(bootstrappedCurve));

        for (auto q : quotes) {
            // create swaps and check they're fair
            Date maturity = calendar.advance(spot, q.n, q.units, convention);

            Schedule s =
                MakeSchedule()
                .from(spot).to(maturity)
                .withTenor(otherIndex->tenor())
                .withCalendar(calendar)
                .withConvention(convention)
                .withRule(DateGeneration::Forward);

            Leg leg1 = OvernightLeg(s, baseIndex)
                .withSpreads(q.basis)
                .withNotionals(100.0);
            Leg leg2 = IborLeg(s, otherIndex)
                .withNotionals(100.0);

            Swap swap(leg1, leg2);
            if (externalDiscountCurve) {
                swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(discountCurve));
            } else {
                swap.setPricingEngine(ext::make_shared<DiscountingSwapEngine>(
                                              Handle<YieldTermStructure>(bootstrappedCurve)));
            }

            Real NPV = swap.NPV();
            Real tolerance = 1e-8;
            if (std::fabs(NPV) > tolerance) {
                BOOST_ERROR("Failed to price fair " << q.n << "-year(s) swap:"
                            << "\n    calculated: " << NPV);
            }
        }
    }

}

BOOST_AUTO_TEST_CASE(testIborIborBaseCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing IBOR-IBOR basis-swap rate helpers (base curve bootstrap)...");

    testIborIborBootstrap(true);
}

BOOST_AUTO_TEST_CASE(testIborIborOtherCurveBootstrap) {
    BOOST_TEST_MESSAGE("Testing IBOR-IBOR basis-swap rate helpers (other curve bootstrap)...");

    testIborIborBootstrap(false);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithoutDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers...");

    testOvernightIborBootstrap(false);
}

BOOST_AUTO_TEST_CASE(testOvernightIborBootstrapWithDiscountCurve) {
    BOOST_TEST_MESSAGE("Testing overnight-IBOR basis-swap rate helpers with external discount curve...");

    testOvernightIborBootstrap(true);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
