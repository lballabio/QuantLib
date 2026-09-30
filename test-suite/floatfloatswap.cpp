/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Shubham Gaur

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
#include <ql/experimental/coupons/swapspreadindex.hpp>
#include <ql/exercise.hpp>
#include <ql/instruments/floatfloatswap.hpp>
#include <ql/instruments/floatfloatswaption.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/pricingengines/swaption/gaussian1dfloatfloatswaptionengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/rebatedexercise.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swap/euriborswap.hpp>
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


BOOST_AUTO_TEST_CASE(testZeroBpsFairSpread) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap fair spread calculation with zero BPS...");

    CommonVars vars;
    vars.nominal = 0.0;
    auto swap = vars.makeSwap(Swap::Payer, 0.0, 0.0);

    BOOST_CHECK(!swap->isExpired());
    BOOST_CHECK_EQUAL(swap->legBPS(0), 0.0);
    BOOST_CHECK_EQUAL(swap->legBPS(1), 0.0);

    BOOST_CHECK_EXCEPTION(
        swap->fairSpread1(), Error,
        ExpectedErrorMessage("fair spread 1 not available"));
    BOOST_CHECK_EXCEPTION(
        swap->fairSpread2(), Error,
        ExpectedErrorMessage("fair spread 2 not available"));
}


BOOST_AUTO_TEST_CASE(testExpiredSwapFairSpread) {

    BOOST_TEST_MESSAGE(
        "Testing float-float swap fair spread calculation for expired swap...");

    CommonVars vars;
    auto swap = vars.makeSwap(Swap::Payer, 0.0, 0.0, 10);

    Settings::instance().evaluationDate() = vars.settlement + Period(20, Years);

    BOOST_CHECK(swap->isExpired());
    BOOST_CHECK_EXCEPTION(
        swap->fairSpread1(), Error,
        ExpectedErrorMessage("fair spread 1 not available"));
    BOOST_CHECK_EXCEPTION(
        swap->fairSpread2(), Error,
        ExpectedErrorMessage("fair spread 2 not available"));
}

BOOST_AUTO_TEST_CASE(testGaussian1dFloatFloatSwaptionAndCalibrationBasket) {
    CommonVars vars;
    const auto model = ext::make_shared<Gsr>(
        vars.termStructure, std::vector<Date>(), std::vector<Real>{0.01}, 0.03, 60.0);
    const Date start = vars.settlement;
    const Date maturity = vars.calendar.advance(start, 5 * Years);
    const Schedule schedule1(start, maturity, 3 * Months, vars.calendar,
                             ModifiedFollowing, ModifiedFollowing,
                             DateGeneration::Forward, false);
    const Schedule schedule2(start, maturity, 6 * Months, vars.calendar,
                             ModifiedFollowing, ModifiedFollowing,
                             DateGeneration::Forward, false);
    const auto swap = ext::make_shared<FloatFloatSwap>(
        Swap::Payer, vars.nominal, vars.nominal, schedule1, vars.index1,
        vars.index1->dayCounter(), schedule2, vars.index2,
        vars.index2->dayCounter());
    const Date firstExercise = vars.calendar.advance(vars.today, 2 * Years);
    const Date secondExercise = vars.calendar.advance(vars.today, 3 * Years);
    const auto exercise = ext::make_shared<BermudanExercise>(
        std::vector<Date>{firstExercise, secondExercise});
    const auto swaption = ext::make_shared<FloatFloatSwaption>(swap, exercise);
    const auto engine = ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
        model, 16, 5.0);
    swaption->setPricingEngine(engine);

    const Real value = swaption->NPV();
    BOOST_CHECK(std::isfinite(value));
    BOOST_CHECK(std::isfinite(swaption->result<Real>("underlyingValue")));

    const auto swaptionVolatility = ext::make_shared<ConstantSwaptionVolatility>(
        vars.settlementDays, vars.calendar, Following, 0.20, Actual365Fixed());
    const auto standardSwapBase = ext::make_shared<EuriborSwapIsdaFixA>(
        5 * Years, vars.termStructure);

    const auto naiveBasket = swaption->calibrationBasket(
        standardSwapBase, swaptionVolatility, BasketGeneratingEngine::Naive);
    BOOST_CHECK_EQUAL(naiveBasket.size(), exercise->dates().size());

    const auto fittedBasket = swaption->calibrationBasket(
        standardSwapBase, swaptionVolatility,
        BasketGeneratingEngine::MaturityStrikeByDeltaGamma);
    BOOST_CHECK_EQUAL(fittedBasket.size(), exercise->dates().size());
}

BOOST_AUTO_TEST_CASE(testGaussian1dFloatFloatSwaptionCoverage) {
    CommonVars vars;
    const auto model = ext::make_shared<Gsr>(
        vars.termStructure, std::vector<Date>(), std::vector<Real>{0.01}, 0.03, 60.0);
    const Date start = vars.settlement;
    const Date maturity = vars.calendar.advance(start, 5 * Years);
    const Schedule schedule1(start, maturity, 3 * Months, vars.calendar,
                             ModifiedFollowing, ModifiedFollowing,
                             DateGeneration::Forward, false);
    const Schedule schedule2(start, maturity, 6 * Months, vars.calendar,
                             ModifiedFollowing, ModifiedFollowing,
                             DateGeneration::Forward, false);
    const Date firstExercise = vars.calendar.advance(vars.today, 2 * Years);
    const Date secondExercise = vars.calendar.advance(vars.today, 3 * Years);
    const auto exercise = ext::make_shared<BermudanExercise>(
        std::vector<Date>{firstExercise, secondExercise});
    const Handle<Quote> oas(ext::make_shared<SimpleQuote>(0.002));

    const auto makeSwap = [&](Swap::Type type,
                              const ext::shared_ptr<InterestRateIndex>& index1,
                              const ext::shared_ptr<InterestRateIndex>& index2,
                              bool exchangePrincipal,
                              bool capAndFloor) {
        const Rate cap = capAndFloor ? 0.01 : Null<Real>();
        const Rate floor = capAndFloor ? 0.001 : Null<Real>();
        return ext::make_shared<FloatFloatSwap>(
            type, vars.nominal, vars.nominal, schedule1, index1,
            index1->dayCounter(), schedule2, index2, index2->dayCounter(),
            exchangePrincipal, exchangePrincipal,
            1.0, 0.001, cap, floor, 1.0, -0.001, cap, floor);
    };

    const auto price = [&](const ext::shared_ptr<FloatFloatSwap>& swap,
                           const ext::shared_ptr<Exercise>& swaptionExercise,
                           Gaussian1dFloatFloatSwaptionEngine::Probabilities probabilities,
                           bool extrapolate,
                           bool flatExtrapolation,
                           bool useOas,
                           std::vector<Real>* probabilitiesOut = nullptr) {
        const auto swaption = ext::make_shared<FloatFloatSwaption>(swap, swaptionExercise);
        const auto engine = ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
            model, 12, 5.0, extrapolate, flatExtrapolation,
            useOas ? oas : Handle<Quote>(), vars.termStructure, false, probabilities);
        swaption->setPricingEngine(engine);
        const Real value = swaption->NPV();
        BOOST_CHECK(std::isfinite(value));
        BOOST_CHECK(std::isfinite(swaption->result<Real>("underlyingValue")));
        if (probabilities != Gaussian1dFloatFloatSwaptionEngine::None) {
            const auto exerciseProbabilities = swaption->result<std::vector<Real>>(
                "probabilities");
            if (probabilitiesOut != nullptr)
                *probabilitiesOut = exerciseProbabilities;
            BOOST_CHECK_EQUAL(exerciseProbabilities.size(), swaptionExercise->dates().size() + 1);
            Real probabilitySum = 0.0;
            for (Real probability : exerciseProbabilities) {
                BOOST_CHECK_GE(probability, 0.0);
                BOOST_CHECK_LE(probability, 1.0 + 1.0e-12);
                probabilitySum += probability;
            }
            BOOST_CHECK_GT(probabilitySum, 0.0);
        }
        return value;
    };

    const auto iborSwap = makeSwap(Swap::Payer, vars.index1, vars.index2, false, false);
    std::vector<Real> flatCallProbabilities;
    const Real nonFlatCall = price(iborSwap, exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, true, false, false);
    const Real flatCall = price(iborSwap, exercise,
        Gaussian1dFloatFloatSwaptionEngine::Naive, true, true, true,
        &flatCallProbabilities);
    const Real flatCallWithoutOas = price(iborSwap, exercise,
        Gaussian1dFloatFloatSwaptionEngine::Naive, true, true, false);
    const Real receiverPrice = price(
        makeSwap(Swap::Receiver, vars.index1, vars.index2, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, true, false, false);
    const Real noExtrapolationPut = price(
        makeSwap(Swap::Receiver, vars.index1, vars.index2, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::Digital, false, false, false);
    const Real extrapolatedDigitalCall = price(iborSwap, exercise,
        Gaussian1dFloatFloatSwaptionEngine::Digital, true, false, false);
    const Real extrapolatedDigitalPut = price(
        makeSwap(Swap::Receiver, vars.index1, vars.index2, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::Digital, true, false, false);
    BOOST_CHECK_SMALL(flatCall - 4.1503974670013793e-06, 1.0e-10);
    BOOST_CHECK_SMALL(flatCallWithoutOas - 4.1437978754246558e-06, 1.0e-10);
    const std::vector<Real> expectedProbabilities{
        0.0, 0.00021123900304688715, 0.99380672505088874};
    BOOST_CHECK_EQUAL(flatCallProbabilities.size(), expectedProbabilities.size());
    for (Size i = 0; i < expectedProbabilities.size(); ++i)
        BOOST_CHECK_SMALL(flatCallProbabilities[i] - expectedProbabilities[i], 1.0e-10);
    BOOST_CHECK_SMALL(nonFlatCall - 4.1426336441813841e-06, 1.0e-10);
    BOOST_CHECK(std::isfinite(nonFlatCall - flatCall));
    BOOST_CHECK(std::isfinite(noExtrapolationPut));
    BOOST_CHECK(std::isfinite(extrapolatedDigitalCall));
    BOOST_CHECK(std::isfinite(extrapolatedDigitalPut));
    BOOST_CHECK_GT(nonFlatCall, 0.0);
    BOOST_CHECK_GT(receiverPrice, 0.0);
    BOOST_CHECK_GT(std::fabs(flatCall - flatCallWithoutOas), 1.0e-12);

    const auto rebateExercise = ext::make_shared<RebatedExercise>(
        *exercise, std::vector<Real>{100.0, 50.0}, 2, vars.calendar);
    const Real rebatedValue = price(iborSwap, rebateExercise,
        Gaussian1dFloatFloatSwaptionEngine::Naive, true, false, true);
    BOOST_CHECK(std::isfinite(rebatedValue));

    const auto exchangedCappedSwap = makeSwap(
        Swap::Payer, vars.index1, vars.index2, true, true);
    BOOST_CHECK(std::isfinite(price(exchangedCappedSwap, exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, true, false, false)));

    const auto cms3m = ext::make_shared<EuriborSwapIsdaFixA>(
        5 * Years, vars.termStructure);
    const auto cms10y = ext::make_shared<EuriborSwapIsdaFixA>(
        10 * Years, vars.termStructure);
    const auto spreadIndex = ext::make_shared<SwapSpreadIndex>(
        "test cms spread", cms10y, cms3m);
    BOOST_CHECK(std::isfinite(price(makeSwap(
        Swap::Payer, cms10y, vars.index2, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, false, false, false)));
    BOOST_CHECK(std::isfinite(price(makeSwap(
        Swap::Receiver, spreadIndex, cms3m, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, true, false, false)));
    BOOST_CHECK(std::isfinite(price(makeSwap(
        Swap::Payer, vars.index1, spreadIndex, false, false), exercise,
        Gaussian1dFloatFloatSwaptionEngine::None, true, false, false)));

    const auto expiredSwaption = ext::make_shared<FloatFloatSwaption>(
        iborSwap, ext::make_shared<EuropeanExercise>(vars.settlement));
    expiredSwaption->setPricingEngine(ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(
        model, 12, 5.0));
    BOOST_CHECK_EQUAL(expiredSwaption->NPV(), 0.0);

    const auto cashSettledSwaption = ext::make_shared<FloatFloatSwaption>(
        iborSwap, ext::make_shared<EuropeanExercise>(firstExercise),
        Settlement::Cash, Settlement::ParYieldCurve);
    cashSettledSwaption->setPricingEngine(
        ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(model, 12, 5.0));
    BOOST_CHECK_EXCEPTION(cashSettledSwaption->NPV(), Error,
        ExpectedErrorMessage("cash settled (ParYieldCurve) swaptions not priced"));
}

BOOST_AUTO_TEST_CASE(testGaussian1dFloatFloatSwaptionCalibrationInitialGuessBoundaries) {
    CommonVars vars;
    const auto model = ext::make_shared<Gsr>(
        vars.termStructure, std::vector<Date>(), std::vector<Real>{0.01}, 0.03, 60.0);
    const auto swaptionVolatility = ext::make_shared<ConstantSwaptionVolatility>(
        vars.settlementDays, vars.calendar, Following, 0.20, Actual365Fixed());
    const auto standardSwapBase = ext::make_shared<EuriborSwapIsdaFixA>(
        5 * Years, vars.termStructure);
    const auto engine = ext::make_shared<Gaussian1dFloatFloatSwaptionEngine>(model, 12, 5.0);

    const Date start = vars.settlement;
    const Date oneYear = vars.calendar.advance(start, 1 * Years);
    const Schedule oneYearSchedule(start, oneYear, 6 * Months, vars.calendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Forward, false);
    const auto oneYearSwap = ext::make_shared<FloatFloatSwap>(
        Swap::Payer, vars.nominal, vars.nominal,
        oneYearSchedule, vars.index2, vars.index2->dayCounter(),
        oneYearSchedule, vars.index2, vars.index2->dayCounter());
    const Date expiryAfterLastReset = vars.calendar.advance(start, 9 * Months);
    const EuropeanExercise europeanExpiry(expiryAfterLastReset);
    const auto rebateExercise = ext::make_shared<RebatedExercise>(
        europeanExpiry, 100.0, 2, vars.calendar);
    const auto noResetSwaption = ext::make_shared<FloatFloatSwaption>(
        oneYearSwap, rebateExercise);
    noResetSwaption->setPricingEngine(engine);
    BOOST_CHECK_EXCEPTION(
        noResetSwaption->calibrationBasket(
            standardSwapBase, swaptionVolatility,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma),
        Error, ExpectedErrorMessage("no leg 1 reset dates remain"));

    const Date fiveYears = vars.calendar.advance(start, 5 * Years);
    const Schedule quarterlySchedule(start, fiveYears, 3 * Months, vars.calendar,
                                     ModifiedFollowing, ModifiedFollowing,
                                     DateGeneration::Forward, false);
    std::vector<Real> notionals1(quarterlySchedule.size() - 1, 0.0);
    notionals1[notionals1.size() - 2] = vars.nominal;
    notionals1.back() = -vars.nominal;
    const std::vector<Real> notionals2(
        quarterlySchedule.size() - 1, vars.nominal);
    const auto cancellingNotionalSwap = ext::make_shared<FloatFloatSwap>(
        Swap::Payer, notionals1, notionals2,
        quarterlySchedule, vars.index1, vars.index1->dayCounter(),
        quarterlySchedule, vars.index1, vars.index1->dayCounter());
    const auto cancellingNotionalSwaption = ext::make_shared<FloatFloatSwaption>(
        cancellingNotionalSwap,
        ext::make_shared<EuropeanExercise>(vars.calendar.advance(vars.today, 2 * Years)));
    cancellingNotionalSwaption->setPricingEngine(engine);
    BOOST_CHECK_EXCEPTION(
        cancellingNotionalSwaption->calibrationBasket(
            standardSwapBase, swaptionVolatility,
            BasketGeneratingEngine::MaturityStrikeByDeltaGamma),
        Error, ExpectedErrorMessage("remaining leg 1 notionals sum to zero"));
}


BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

