/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Hiroto Ogawa

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/perpetualfutures.hpp>
#include <ql/pricingengines/futures/discountingperpetualfuturesengine.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(PerpetualFuturesTests)

#undef REPORT_FAILURE
#define REPORT_FAILURE(greekName, payoffType, fundingType, fundingFreq, s, r, q, k, i_diff, today, \
                       expected, calculated, relError, tolerance)                               \
    BOOST_FAIL(payoffType                                                                    \
               << " perpetual futures with " << fundingType << " funding type:\n"  \
               << "    spot value:                      " << s << "\n"                                      \
               << "    risk-free rate:                  " << r << "\n"                            \
               << "    asset yield:                     " << q << "\n"                            \
               << "    funding rate:                    " << k << "\n"                            \
               << "    interest rate diffierential:     " << i_diff << "\n"                       \
               << "    funding frequency:               " << fundingFreq << "\n"                       \
               << "    reference date:                  " << today << "\n"                                  \
               << "    expected   " << greekName << ": " << expected << "\n"                 \
               << "    calculated " << greekName << ": " << calculated << "\n"               \
               << "    rel error: " << relError << "\n"                                  \
               << "    tolerance: " << tolerance << "\n");

struct PerpetualFuturesData {
    PerpetualFutures::PayoffType payoffType;
    PerpetualFutures::FundingType fundingType;
    Period fundingFreq;
    Real s;       // spot
    Rate r;       // risk-free rate
    Rate q;       // asset yield
    Rate k;       // funding rate
    Rate i_diff;  // interest rate differential
    Real relTol;  // relative tolerance
};


BOOST_AUTO_TEST_CASE(testPerpetualFuturesValues) {

    BOOST_TEST_MESSAGE("Testing perpetual futures value aginast analytic form for constant parameters...");

    PerpetualFuturesData values[] = {
        // Discrete time
        {PerpetualFutures::Linear,  PerpetualFutures::AHJ,     Period(3, Months), 10000., 0.04, 0.02, 0.01, 0.005, 1.e-6},
        {PerpetualFutures::Linear,  PerpetualFutures::AHJ_alt, Period(3, Months), 10000., 0.04, 0.02, 0.01, 0.005, 1.e-6},
        {PerpetualFutures::Inverse, PerpetualFutures::AHJ,     Period(3, Months), 10000., 0.04, 0.02, 0.01, 0.005, 1.e-6},
        {PerpetualFutures::Inverse, PerpetualFutures::AHJ_alt, Period(3, Months), 10000., 0.04, 0.02, 0.01, 0.005, 1.e-6},
        {PerpetualFutures::Linear,  PerpetualFutures::AHJ,     Period(3, Months), 10000., 0.04, 0.02, 0.01, 0.005, 1.e-6},
        // Continuous time
        {PerpetualFutures::Linear,  PerpetualFutures::AHJ,     Period(0, Months), 10000., 0.04, 0.02, 0.2,  0.005, 1.e-6},
        {PerpetualFutures::Inverse, PerpetualFutures::AHJ,     Period(0, Months), 10000., 0.04, 0.02, 0.2,  0.005, 1.e-6},

    };

    DayCounter dc = ActualActual(ActualActual::ISDA);
    Calendar cal = NullCalendar();
    Date today = Date::todaysDate();

    for (auto& value : values) {
        PerpetualFutures trade(value.payoffType, value.fundingType, value.fundingFreq, cal, dc);
        Handle<YieldTermStructure> domCurve(flatRate(today, value.r, dc));
        Handle<YieldTermStructure> forCurve(flatRate(today, value.q, dc));
        Handle<Quote> spot(ext::shared_ptr<SimpleQuote>(new SimpleQuote(value.s)));
        Array fundingTimes(1, 0.), fundingRates(1, value.k), interestRateDiffs(1, value.i_diff);
        ext::shared_ptr<PricingEngine> engine(new DiscountingPerpetualFuturesEngine(
            domCurve, forCurve, spot, fundingTimes, fundingRates, interestRateDiffs));
        trade.setPricingEngine(engine);
        Real calculated = trade.NPV();

        // analytic
        Real dt = 0.;
        switch (value.fundingFreq.units()) {
            case Years:
                dt = (Real)value.fundingFreq.length();
                break;
            case Months:
                dt = (Real)value.fundingFreq.length() / 12.;
                break;
            case Weeks:
                dt = (Real)value.fundingFreq.length() / 365. * 7.;
                break;
            case Days:
                dt = (Real)value.fundingFreq.length() / 365.;
                break;
            case Hours:
                dt = (Real)value.fundingFreq.length() / 365. / 24.;
                break;
            case Minutes:
                dt = (Real)value.fundingFreq.length() / 365. / 24. / 60.;
                break;
            case Seconds:
                dt = (Real)value.fundingFreq.length() / 365. / 24. / 60. / 60.;
                break;
            default:
                QL_FAIL("Unknown fundingFrequency unit");
        }
        Real expected = 0.;
        // Discrete time
        if (value.fundingFreq.length() > 0) {
            if (value.payoffType == PerpetualFutures::Linear) {
                if (value.fundingType == PerpetualFutures::AHJ) {
                    expected =
                        value.s * (value.k - value.i_diff) * exp(value.q * dt) /
                        (exp(value.q * dt) - exp(value.r * dt) + value.k * exp(value.q * dt));
                } else if (value.fundingType == PerpetualFutures::AHJ_alt) {
                    expected =
                        value.s * (value.k - value.i_diff) * exp(value.r * dt) /
                        (exp(value.q * dt) - exp(value.r * dt) + value.k * exp(value.r * dt));
                }
            } else if (value.payoffType == PerpetualFutures::Inverse) {
                if (value.fundingType == PerpetualFutures::AHJ) {
                    expected =
                        value.s *
                        (exp(value.r * dt) - exp(value.q * dt) + value.k * exp(value.r * dt)) /
                        (value.k - value.i_diff) / exp(value.r * dt);
                } else if (value.fundingType == PerpetualFutures::AHJ_alt) {
                    expected =
                        value.s *
                        (exp(value.r * dt) - exp(value.q * dt) + value.k * exp(value.q * dt)) /
                        (value.k - value.i_diff) / exp(value.q * dt);
                }
            }
        } else {
            // Continuous time
            if (value.payoffType == PerpetualFutures::Linear) {
                expected = value.s * (value.k - value.i_diff) / (value.q - value.r + value.k);
            } else if (value.payoffType == PerpetualFutures::Inverse) {
                expected = value.s * (value.r - value.q + value.k) / (value.k - value.i_diff);
            }
        }
        Real relError = std::fabs(calculated / expected - 1.);
        if (relError > value.relTol) {
            REPORT_FAILURE("value", value.payoffType, value.fundingType, value.fundingFreq, value.s,
                           value.r, value.q, value.k, value.i_diff, today, expected, calculated,
                           relError, value.relTol);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
