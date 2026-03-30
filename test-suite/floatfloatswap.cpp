/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Shubham

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
#include <ql/instruments/floatfloatswap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(FloatFloatSwapTests)

struct CommonVars {
    Date today, settlement;
    Real nominal;
    Calendar calendar;
    RelinkableHandle<YieldTermStructure> termStructure;
    ext::shared_ptr<IborIndex> index1, index2;
    Natural settlementDays;

    ext::shared_ptr<FloatFloatSwap>
    makeSwap(Swap::Type type,
             Spread spread1,
             Spread spread2,
             Integer lengthInYears = 10) const {

        Date maturity = calendar.advance(settlement, lengthInYears, Years,
                                         ModifiedFollowing);

        Schedule schedule1(settlement, maturity, index1->tenor(),
                           calendar, ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Forward, false);

        Schedule schedule2(settlement, maturity, index2->tenor(),
                           calendar, ModifiedFollowing, ModifiedFollowing,
                           DateGeneration::Forward, false);

        auto swap = ext::make_shared<FloatFloatSwap>(
            type, nominal, nominal,
            schedule1, index1, index1->dayCounter(),
            schedule2, index2, index2->dayCounter(),
            false, false,
            1.0, spread1, Null<Real>(), Null<Real>(),
            1.0, spread2, Null<Real>(), Null<Real>());

        auto engine = ext::make_shared<DiscountingSwapEngine>(termStructure);
        swap->setPricingEngine(engine);

        auto pricer = ext::make_shared<BlackIborCouponPricer>();
        setCouponPricer(swap->leg1(), pricer);
        setCouponPricer(swap->leg2(), pricer);

        return swap;
    }

    CommonVars() {
        settlementDays = 2;
        nominal = 100.0;
        calendar = TARGET();
        today = calendar.adjust(Settings::instance().evaluationDate());
        settlement = calendar.advance(today, settlementDays, Days);
        termStructure.linkTo(
            flatRate(settlement, 0.05, Actual365Fixed()));
        index1 = ext::make_shared<Euribor>(3 * Months, termStructure);
        index2 = ext::make_shared<Euribor>(6 * Months, termStructure);
    }
};


BOOST_AUTO_TEST_CASE(testFairSpread1) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap calculation of fair spread on leg 1...");

    CommonVars vars;

    Swap::Type types[] = { Swap::Payer, Swap::Receiver };
    Spread spread2Values[] = { -0.002, 0.0, 0.002, 0.005 };

    for (auto type : types) {
        for (Real spread2 : spread2Values) {

            auto swap = vars.makeSwap(type, 0.0, spread2);
            Spread fair = swap->fairSpread1();

            auto swap2 = vars.makeSwap(type, fair, spread2);
            if (std::fabs(swap2->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with fair spread on leg 1:\n"
                            << "    type: "
                            << ((type == Swap::Payer) ? "Payer" : "Receiver")
                            << "\n"
                            << "    spread on leg 2: "
                            << io::rate(spread2) << "\n"
                            << "    fair spread on leg 1: "
                            << io::rate(fair) << "\n"
                            << "    swap NPV: " << swap2->NPV());
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(testFairSpread2) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap calculation of fair spread on leg 2...");

    CommonVars vars;

    Swap::Type types[] = { Swap::Payer, Swap::Receiver };
    Spread spread1Values[] = { -0.002, 0.0, 0.002, 0.005 };

    for (auto type : types) {
        for (Real spread1 : spread1Values) {

            auto swap = vars.makeSwap(type, spread1, 0.0);
            Spread fair = swap->fairSpread2();

            auto swap2 = vars.makeSwap(type, spread1, fair);
            if (std::fabs(swap2->NPV()) > 1.0e-10) {
                BOOST_ERROR("recalculating with fair spread on leg 2:\n"
                            << "    type: "
                            << ((type == Swap::Payer) ? "Payer" : "Receiver")
                            << "\n"
                            << "    spread on leg 1: "
                            << io::rate(spread1) << "\n"
                            << "    fair spread on leg 2: "
                            << io::rate(fair) << "\n"
                            << "    swap NPV: " << swap2->NPV());
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(testPayerReceiverSymmetry) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap payer/receiver NPV symmetry...");

    CommonVars vars;

    Spread spread1 = 0.001;
    Spread spread2 = 0.003;

    auto payer = vars.makeSwap(Swap::Payer, spread1, spread2);
    auto receiver = vars.makeSwap(Swap::Receiver, spread1, spread2);

    Real tolerance = 1.0e-10;
    if (std::fabs(payer->NPV() + receiver->NPV()) > tolerance) {
        BOOST_ERROR("payer and receiver NPVs do not cancel:\n"
                    << "    payer NPV:    " << payer->NPV() << "\n"
                    << "    receiver NPV: " << receiver->NPV() << "\n"
                    << "    sum:          "
                    << (payer->NPV() + receiver->NPV()));
    }
}


BOOST_AUTO_TEST_CASE(testFairSpreadPayerReceiverConsistency) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap fair spread consistency "
        "between payer and receiver...");

    CommonVars vars;

    Spread spread2 = 0.002;

    auto payer = vars.makeSwap(Swap::Payer, 0.0, spread2);
    auto receiver = vars.makeSwap(Swap::Receiver, 0.0, spread2);

    Spread fairPayer = payer->fairSpread1();
    Spread fairReceiver = receiver->fairSpread1();

    Real tolerance = 1.0e-10;
    if (std::fabs(fairPayer - fairReceiver) > tolerance) {
        BOOST_ERROR("fair spread on leg 1 differs between payer and receiver:\n"
                    << "    payer fair spread 1:    "
                    << io::rate(fairPayer) << "\n"
                    << "    receiver fair spread 1: "
                    << io::rate(fairReceiver));
    }

    auto payer2 = vars.makeSwap(Swap::Payer, spread2, 0.0);
    auto receiver2 = vars.makeSwap(Swap::Receiver, spread2, 0.0);

    fairPayer = payer2->fairSpread2();
    fairReceiver = receiver2->fairSpread2();

    if (std::fabs(fairPayer - fairReceiver) > tolerance) {
        BOOST_ERROR("fair spread on leg 2 differs between payer and receiver:\n"
                    << "    payer fair spread 2:    "
                    << io::rate(fairPayer) << "\n"
                    << "    receiver fair spread 2: "
                    << io::rate(fairReceiver));
    }
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
