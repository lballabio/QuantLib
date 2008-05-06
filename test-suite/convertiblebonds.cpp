/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 StatPro Italia srl
 Copyright (C) 2007 Ferdinando Ametrano

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

#include "convertiblebonds.hpp"
#include "utilities.hpp"
#include <ql/instruments/bonds/convertiblebond.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/hybrid/binomialconvertibleengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/forwardspreadedtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    struct CommonVars {
        // global data
        Date today, issueDate, maturityDate;
        Calendar calendar;
        DayCounter dayCounter;
        Frequency frequency;
        Natural settlementDays;

        RelinkableHandle<Quote> underlying;
        RelinkableHandle<YieldTermStructure> dividendYield, riskFreeRate;
        RelinkableHandle<BlackVolTermStructure> volatility;
        boost::shared_ptr<BlackScholesMertonProcess> process;

        RelinkableHandle<Quote> creditSpread;

        CallabilitySchedule no_callability;
        DividendSchedule no_dividends;

        Real faceAmount, redemption, conversionRatio;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {
            calendar = TARGET();

            today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;

            dayCounter = Actual360();
            frequency = Annual;
            settlementDays = 3;

            issueDate = calendar.advance(today,2,Days);
            maturityDate = calendar.advance(issueDate, 10, Years);
            // reset to avoid inconsistencies as the schedule is backwards
            issueDate = calendar.advance(maturityDate, -10, Years);

            underlying.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(50.0)));
            dividendYield.linkTo(flatRate(today, 0.02, dayCounter));
            riskFreeRate.linkTo(flatRate(today, 0.05, dayCounter));
            volatility.linkTo(flatVol(today, 0.15, dayCounter));

            process = boost::shared_ptr<BlackScholesMertonProcess>(
                    new BlackScholesMertonProcess(underlying, dividendYield,
                                                  riskFreeRate, volatility));

            creditSpread.linkTo(
                            boost::shared_ptr<Quote>(new SimpleQuote(0.005)));

            // it fails with 1000000
            // faceAmount = 1000000.0;
            faceAmount = 100.0;
            redemption = 100.0;
            conversionRatio = redemption/underlying->value();
        }
    };

}


void ConvertibleBondTest::testBond() {

    /* when deeply out-of-the-money, the value of the convertible bond
       should equal that of the underlying plain-vanilla bond. */

    BOOST_MESSAGE(
       "Testing out-of-the-money convertible bonds against vanilla bonds...");

    CommonVars vars;

    vars.conversionRatio = 1.0e-16;

    boost::shared_ptr<Exercise> euExercise(
                                     new EuropeanExercise(vars.maturityDate));
    boost::shared_ptr<Exercise> amExercise(
                                     new AmericanExercise(vars.issueDate,
                                                          vars.maturityDate));

    Size timeSteps = 1001;
    boost::shared_ptr<PricingEngine> engine(
        new BinomialConvertibleEngine<CoxRossRubinstein>(vars.process,
                                                         timeSteps));

    Handle<YieldTermStructure> discountCurve(
         boost::shared_ptr<YieldTermStructure>(
             new ForwardSpreadedTermStructure(vars.riskFreeRate,
                                              vars.creditSpread)));

    // zero-coupon

    Schedule schedule = MakeSchedule(vars.issueDate, vars.maturityDate,
                                     Period(Once), vars.calendar,
                                     Following).backwards();

    ConvertibleZeroCouponBond euZero(euExercise, vars.conversionRatio,
                                     vars.no_dividends, vars.no_callability,
                                     vars.creditSpread,
                                     vars.issueDate, vars.settlementDays,
                                     vars.dayCounter, schedule,
                                     vars.redemption);
    euZero.setPricingEngine(engine);

    ConvertibleZeroCouponBond amZero(amExercise, vars.conversionRatio,
                                     vars.no_dividends, vars.no_callability,
                                     vars.creditSpread,
                                     vars.issueDate, vars.settlementDays,
                                     vars.dayCounter, schedule,
                                     vars.redemption);
    amZero.setPricingEngine(engine);

    ZeroCouponBond zero(vars.settlementDays, vars.calendar,
                        100.0, vars.maturityDate,
                        Following, vars.redemption, vars.issueDate);

    boost::shared_ptr<PricingEngine> bondEngine(
                                    new DiscountingBondEngine(discountCurve));
    zero.setPricingEngine(bondEngine);

    Real tolerance = 1.0e-2 * (vars.faceAmount/100.0);

    Real error = std::fabs(euZero.NPV()-zero.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce zero-coupon bond price:"
                    << "\n    calculated: " << euZero.NPV()
                    << "\n    expected:   " << zero.NPV()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amZero.NPV()-zero.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce zero-coupon bond price:"
                    << "\n    calculated: " << amZero.NPV()
                    << "\n    expected:   " << zero.NPV()
                    << "\n    error:      " << error);
    }

    // coupon

    std::vector<Rate> coupons(1, 0.05);

    schedule = MakeSchedule(vars.issueDate, vars.maturityDate,
                            Period(vars.frequency), vars.calendar,
                            Following).backwards();

    ConvertibleFixedCouponBond euFixed(euExercise, vars.conversionRatio,
                                       vars.no_dividends, vars.no_callability,
                                       vars.creditSpread,
                                       vars.issueDate, vars.settlementDays,
                                       coupons, vars.dayCounter,
                                       schedule, vars.redemption);
    euFixed.setPricingEngine(engine);

    ConvertibleFixedCouponBond amFixed(amExercise, vars.conversionRatio,
                                       vars.no_dividends, vars.no_callability,
                                       vars.creditSpread,
                                       vars.issueDate, vars.settlementDays,
                                       coupons, vars.dayCounter,
                                       schedule, vars.redemption);
    amFixed.setPricingEngine(engine);

    FixedRateBond fixed(vars.settlementDays, vars.faceAmount, schedule,
                        coupons, vars.dayCounter, Following,
                        vars.redemption, vars.issueDate);

    fixed.setPricingEngine(bondEngine);

    tolerance = 2.0e-2 * (vars.faceAmount/100.0);

    error = std::fabs(euFixed.NPV()-fixed.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce fixed-coupon bond price:"
                    << "\n    calculated: " << euFixed.NPV()
                    << "\n    expected:   " << fixed.NPV()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amFixed.NPV()-fixed.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce fixed-coupon bond price:"
                    << "\n    calculated: " << amFixed.NPV()
                    << "\n    expected:   " << fixed.NPV()
                    << "\n    error:      " << error);
    }

    // floating-rate

    boost::shared_ptr<IborIndex> index(new Euribor1Y(discountCurve));
    Natural fixingDays = 2;
    std::vector<Real> gearings(1, 1.0);
    std::vector<Rate> spreads;

    ConvertibleFloatingRateBond euFloating(euExercise, vars.conversionRatio,
                                           vars.no_dividends, vars.no_callability,
                                           vars.creditSpread,
                                           vars.issueDate, vars.settlementDays,
                                           index, fixingDays, spreads,
                                           vars.dayCounter, schedule,
                                           vars.redemption);
    euFloating.setPricingEngine(engine);

    ConvertibleFloatingRateBond amFloating(amExercise, vars.conversionRatio,
                                           vars.no_dividends, vars.no_callability,
                                           vars.creditSpread,
                                           vars.issueDate, vars.settlementDays,
                                           index, fixingDays, spreads,
                                           vars.dayCounter, schedule,
                                           vars.redemption);
    amFloating.setPricingEngine(engine);

    boost::shared_ptr<IborCouponPricer> pricer(new
        BlackIborCouponPricer(Handle<OptionletVolatilityStructure>()));

    Schedule floatSchedule(vars.issueDate, vars.maturityDate,
                           Period(vars.frequency),
                           vars.calendar, Following, Following,
                           DateGeneration::Backward, false);

    FloatingRateBond floating(vars.settlementDays, vars.faceAmount, floatSchedule,
                              index, vars.dayCounter, Following, fixingDays,
                              gearings, spreads,
                              std::vector<Rate>(), std::vector<Rate>(),
                              false,
                              vars.redemption, vars.issueDate);

    floating.setPricingEngine(bondEngine);
    setCouponPricer(floating.cashflows(),pricer);

    tolerance = 2.0e-2 * (vars.faceAmount/100.0);

    error = std::fabs(euFloating.NPV()-floating.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce floating-rate bond price:"
                    << "\n    calculated: " << euFloating.NPV()
                    << "\n    expected:   " << floating.NPV()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amFloating.NPV()-floating.NPV());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce floating-rate bond price:"
                    << "\n    calculated: " << amFloating.NPV()
                    << "\n    expected:   " << floating.NPV()
                    << "\n    error:      " << error);
    }
}

void ConvertibleBondTest::testOption() {

    /* a zero-coupon convertible bond with no credit spread is
       equivalent to a call option. */

    BOOST_MESSAGE(
       "Testing zero-coupon convertible bonds against vanilla option...");

    CommonVars vars;

    boost::shared_ptr<Exercise> euExercise(new
                                    EuropeanExercise(vars.maturityDate));

    vars.settlementDays = 0;

    Size timeSteps = 1001;
    boost::shared_ptr<PricingEngine> engine(
        new BinomialConvertibleEngine<CoxRossRubinstein>(vars.process,
                                                         timeSteps));
    boost::shared_ptr<PricingEngine> vanillaEngine(
            new BinomialVanillaEngine<CoxRossRubinstein>(vars.process,
                                                         timeSteps));

    vars.creditSpread.linkTo(boost::shared_ptr<Quote>(new SimpleQuote(0.0)));

    Real conversionStrike = vars.redemption/vars.conversionRatio;
    boost::shared_ptr<StrikedTypePayoff> payoff(
                      new PlainVanillaPayoff(Option::Call, conversionStrike));

    Schedule schedule = MakeSchedule(vars.issueDate, vars.maturityDate,
                                     Period(Once), vars.calendar,
                                     Following).backwards();

    ConvertibleZeroCouponBond euZero(euExercise, vars.conversionRatio,
                                     vars.no_dividends, vars.no_callability,
                                     vars.creditSpread,
                                     vars.issueDate, vars.settlementDays,
                                     vars.dayCounter, schedule,
                                     vars.redemption);
    euZero.setPricingEngine(engine);

    VanillaOption euOption(payoff, euExercise);
    euOption.setPricingEngine(vanillaEngine);

    Real tolerance = 5.0e-2 * (vars.faceAmount/100.0);

    Real expected = vars.faceAmount/100.0 *
        (vars.redemption * vars.riskFreeRate->discount(vars.maturityDate)
         + vars.conversionRatio* euOption.NPV());
    Real error = std::fabs(euZero.NPV()-expected);
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce plain-option price:"
                    << "\n    calculated: " << euZero.NPV()
                    << "\n    expected:   " << expected
                    << "\n    error:      " << error);
    }
}

test_suite* ConvertibleBondTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Convertible bond tests");

    suite->add(QUANTLIB_TEST_CASE(&ConvertibleBondTest::testBond));
    suite->add(QUANTLIB_TEST_CASE(&ConvertibleBondTest::testOption));

    return suite;
}
