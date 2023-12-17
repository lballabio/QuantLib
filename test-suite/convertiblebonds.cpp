/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008, 2009 StatPro Italia srl
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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/bonds/convertiblebonds.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/bond/binomialconvertibleengine.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/forwardcurve.hpp>
#include <ql/termstructures/yield/forwardspreadedtermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(ConvertibleBondTests)

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
    ext::shared_ptr<BlackScholesMertonProcess> process;

    RelinkableHandle<Quote> creditSpread;

    CallabilitySchedule no_callability;

    Real faceAmount, redemption, conversionRatio;

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

        underlying.linkTo(ext::make_shared<SimpleQuote>(50.0));
        dividendYield.linkTo(flatRate(today, 0.02, dayCounter));
        riskFreeRate.linkTo(flatRate(today, 0.05, dayCounter));
        volatility.linkTo(flatVol(today, 0.15, dayCounter));

        process = ext::make_shared<BlackScholesMertonProcess>(
                    underlying, dividendYield, riskFreeRate, volatility);

        creditSpread.linkTo(ext::make_shared<SimpleQuote>(0.005));

        // it fails with 1000000
        // faceAmount = 1000000.0;
        faceAmount = 100.0;
        redemption = 100.0;
        conversionRatio = redemption/underlying->value();
    }
};


BOOST_AUTO_TEST_CASE(testBond) {

    /* when deeply out-of-the-money, the value of the convertible bond
       should equal that of the underlying plain-vanilla bond. */

    BOOST_TEST_MESSAGE(
       "Testing out-of-the-money convertible bonds against vanilla bonds...");

    CommonVars vars;

    vars.conversionRatio = 1.0e-16;

    ext::shared_ptr<Exercise> euExercise =
        ext::make_shared<EuropeanExercise>(vars.maturityDate);
    ext::shared_ptr<Exercise> amExercise =
        ext::make_shared<AmericanExercise>(vars.issueDate,
                                             vars.maturityDate);

    Size timeSteps = 1001;
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<BinomialConvertibleEngine<CoxRossRubinstein> >(vars.process,
            timeSteps, 
            vars.creditSpread);

    Handle<YieldTermStructure> discountCurve(
         ext::make_shared<ForwardSpreadedTermStructure>(vars.riskFreeRate,
                                                        vars.creditSpread));

    // zero-coupon

    Schedule schedule =
        MakeSchedule().from(vars.issueDate)
                      .to(vars.maturityDate)
                      .withFrequency(Once)
                      .withCalendar(vars.calendar)
                      .backwards();

    ConvertibleZeroCouponBond euZero(euExercise, vars.conversionRatio,
                                     vars.no_callability,
                                     vars.issueDate, vars.settlementDays,
                                     vars.dayCounter, schedule,
                                     vars.redemption);
    euZero.setPricingEngine(engine);

    ConvertibleZeroCouponBond amZero(amExercise, vars.conversionRatio,
                                     vars.no_callability,
                                     vars.issueDate, vars.settlementDays,
                                     vars.dayCounter, schedule,
                                     vars.redemption);
    amZero.setPricingEngine(engine);

    ZeroCouponBond zero(vars.settlementDays, vars.calendar,
                        100.0, vars.maturityDate,
                        Following, vars.redemption, vars.issueDate);

    ext::shared_ptr<PricingEngine> bondEngine =
        ext::make_shared<DiscountingBondEngine>(discountCurve);
    zero.setPricingEngine(bondEngine);

    Real tolerance = 1.0e-2 * (vars.faceAmount/100.0);

    Real error = std::fabs(euZero.NPV()-zero.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce zero-coupon bond price:"
                    << "\n    calculated: " << euZero.NPV()
                    << "\n    expected:   " << zero.settlementValue()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amZero.NPV()-zero.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce zero-coupon bond price:"
                    << "\n    calculated: " << amZero.NPV()
                    << "\n    expected:   " << zero.settlementValue()
                    << "\n    error:      " << error);
    }

    // coupon

    std::vector<Rate> coupons(1, 0.05);

    schedule = MakeSchedule().from(vars.issueDate)
                             .to(vars.maturityDate)
                             .withFrequency(vars.frequency)
                             .withCalendar(vars.calendar)
                             .backwards();

    ConvertibleFixedCouponBond euFixed(euExercise, vars.conversionRatio,
                                       vars.no_callability,
                                       vars.issueDate, vars.settlementDays,
                                       coupons, vars.dayCounter,
                                       schedule, vars.redemption);
    euFixed.setPricingEngine(engine);

    ConvertibleFixedCouponBond amFixed(amExercise, vars.conversionRatio,
                                       vars.no_callability,
                                       vars.issueDate, vars.settlementDays,
                                       coupons, vars.dayCounter,
                                       schedule, vars.redemption);
    amFixed.setPricingEngine(engine);

    FixedRateBond fixed(vars.settlementDays, vars.faceAmount, schedule,
                        coupons, vars.dayCounter, Following,
                        vars.redemption, vars.issueDate);

    fixed.setPricingEngine(bondEngine);

    tolerance = 2.0e-2 * (vars.faceAmount/100.0);

    error = std::fabs(euFixed.NPV()-fixed.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce fixed-coupon bond price:"
                    << "\n    calculated: " << euFixed.NPV()
                    << "\n    expected:   " << fixed.settlementValue()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amFixed.NPV()-fixed.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce fixed-coupon bond price:"
                    << "\n    calculated: " << amFixed.NPV()
                    << "\n    expected:   " << fixed.settlementValue()
                    << "\n    error:      " << error);
    }

    // floating-rate

    ext::shared_ptr<IborIndex> index =
        ext::make_shared<Euribor1Y>(discountCurve);
    Natural fixingDays = 2;
    std::vector<Real> gearings(1, 1.0);
    std::vector<Rate> spreads;

    ConvertibleFloatingRateBond euFloating(euExercise, vars.conversionRatio,
                                           vars.no_callability,
                                           vars.issueDate, vars.settlementDays,
                                           index, fixingDays, spreads,
                                           vars.dayCounter, schedule,
                                           vars.redemption);
    euFloating.setPricingEngine(engine);

    ConvertibleFloatingRateBond amFloating(amExercise, vars.conversionRatio,
                                           vars.no_callability,
                                           vars.issueDate, vars.settlementDays,
                                           index, fixingDays, spreads,
                                           vars.dayCounter, schedule,
                                           vars.redemption);
    amFloating.setPricingEngine(engine);

    ext::shared_ptr<IborCouponPricer> pricer =
        ext::make_shared<BlackIborCouponPricer>(
            Handle<OptionletVolatilityStructure>());

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

    error = std::fabs(euFloating.NPV()-floating.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce floating-rate bond price:"
                    << "\n    calculated: " << euFloating.NPV()
                    << "\n    expected:   " << floating.settlementValue()
                    << "\n    error:      " << error);
    }

    error = std::fabs(amFloating.NPV()-floating.settlementValue());
    if (error > tolerance) {
        BOOST_ERROR("failed to reproduce floating-rate bond price:"
                    << "\n    calculated: " << amFloating.NPV()
                    << "\n    expected:   " << floating.settlementValue()
                    << "\n    error:      " << error);
    }
}

BOOST_AUTO_TEST_CASE(testOption) {

    /* a zero-coupon convertible bond with no credit spread is
       equivalent to a call option. */

    BOOST_TEST_MESSAGE(
       "Testing zero-coupon convertible bonds against vanilla option...");

    CommonVars vars;

    ext::shared_ptr<Exercise> euExercise =
        ext::make_shared<EuropeanExercise>(vars.maturityDate);

    vars.settlementDays = 0;

    Size timeSteps = 2001;
    ext::shared_ptr<PricingEngine> engine =
        ext::make_shared<BinomialConvertibleEngine<CoxRossRubinstein> >(
            vars.process, timeSteps, vars.creditSpread);
    ext::shared_ptr<PricingEngine> vanillaEngine =
        ext::make_shared<BinomialVanillaEngine<CoxRossRubinstein> >(
            vars.process, timeSteps);

    vars.creditSpread.linkTo(ext::make_shared<SimpleQuote>(0.0));

    Real conversionStrike = vars.redemption/vars.conversionRatio;
    ext::shared_ptr<StrikedTypePayoff> payoff =
        ext::make_shared<PlainVanillaPayoff>(Option::Call, conversionStrike);

    Schedule schedule = MakeSchedule().from(vars.issueDate)
                                      .to(vars.maturityDate)
                                      .withFrequency(Once)
                                      .withCalendar(vars.calendar)
                                      .backwards();

    ConvertibleZeroCouponBond euZero(euExercise, vars.conversionRatio,
                                     vars.no_callability,
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
                    << "\n    error:      " << error
                    << "\n    tolerance:      " << tolerance);
    }
}

BOOST_AUTO_TEST_CASE(testRegression) {

    BOOST_TEST_MESSAGE(
       "Testing fixed-coupon convertible bond in known regression case...");

    Date today = Date(23, December, 2008);
    Date tomorrow = today + 1;

    Settings::instance().evaluationDate() = tomorrow;

    Handle<Quote> u(ext::make_shared<SimpleQuote>(2.9084382818797443));

    std::vector<Date> dates(25);
    std::vector<Rate> forwards(25);
    dates[0] = Date(29,December,2008);   forwards[0] = 0.0025999342800;
    dates[1] = Date(5,January,2009);     forwards[1] = 0.0025999342800;
    dates[2] = Date(29,January,2009);    forwards[2] = 0.0053123275500;
    dates[3] = Date(27,February,2009);   forwards[3] = 0.0197049598721;
    dates[4] = Date(30,March,2009);      forwards[4] = 0.0220524845296;
    dates[5] = Date(29,June,2009);       forwards[5] = 0.0217076395643;
    dates[6] = Date(29,December,2009);   forwards[6] = 0.0230349627478;
    dates[7] = Date(29,December,2010);   forwards[7] = 0.0087631647476;
    dates[8] = Date(29,December,2011);   forwards[8] = 0.0219084299499;
    dates[9] = Date(31,December,2012);   forwards[9] = 0.0244798766219;
    dates[10] = Date(30,December,2013);  forwards[10] = 0.0267885498456;
    dates[11] = Date(29,December,2014);  forwards[11] = 0.0266922867562;
    dates[12] = Date(29,December,2015);  forwards[12] = 0.0271052126386;
    dates[13] = Date(29,December,2016);  forwards[13] = 0.0268829891648;
    dates[14] = Date(29,December,2017);  forwards[14] = 0.0264594744498;
    dates[15] = Date(31,December,2018);  forwards[15] = 0.0273450367424;
    dates[16] = Date(30,December,2019);  forwards[16] = 0.0294852614749;
    dates[17] = Date(29,December,2020);  forwards[17] = 0.0285556119719;
    dates[18] = Date(29,December,2021);  forwards[18] = 0.0305557764659;
    dates[19] = Date(29,December,2022);  forwards[19] = 0.0292244738422;
    dates[20] = Date(29,December,2023);  forwards[20] = 0.0263917004194;
    dates[21] = Date(29,December,2028);  forwards[21] = 0.0239626970243;
    dates[22] = Date(29,December,2033);  forwards[22] = 0.0216417108090;
    dates[23] = Date(29,December,2038);  forwards[23] = 0.0228343838422;
    dates[24] = Date(31,December,2199);  forwards[24] = 0.0228343838422;

    Handle<YieldTermStructure> r(
              ext::make_shared<ForwardCurve>(dates, forwards, Actual360()));

    Handle<BlackVolTermStructure> sigma(ext::make_shared<BlackConstantVol>(
                                 tomorrow, NullCalendar(), 21.685235548092248,
                                 Thirty360(Thirty360::BondBasis)));

    ext::shared_ptr<BlackProcess> process =
        ext::make_shared<BlackProcess>(u,r,sigma);

    Handle<Quote> spread(ext::make_shared<SimpleQuote>(0.11498700678012874));

    Date issueDate(23, July, 2008);
    Date maturityDate(1, August, 2013);
    Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);
    Schedule schedule = MakeSchedule().from(issueDate)
                                      .to(maturityDate)
                                      .withTenor(6*Months)
                                      .withCalendar(calendar)
                                      .withConvention(Unadjusted);
    Natural settlementDays = 3;
    ext::shared_ptr<Exercise> exercise =
        ext::make_shared<EuropeanExercise>(maturityDate);
    Real conversionRatio = 100.0/20.3175;
    std::vector<Rate> coupons(schedule.size()-1, 0.05);
    DayCounter dayCounter = Thirty360(Thirty360::BondBasis);
    CallabilitySchedule no_callability;
    DividendSchedule no_dividends;
    Real redemption = 100.0;

    ConvertibleFixedCouponBond bond(exercise, conversionRatio,
                                    no_callability,
                                    issueDate, settlementDays,
                                    coupons, dayCounter,
                                    schedule, redemption);
    bond.setPricingEngine(ext::make_shared<BinomialConvertibleEngine<CoxRossRubinstein> >(
        process, 600, spread, no_dividends));

    try {
        Real x = bond.NPV();  // should throw; if not, an INF was not detected.
        BOOST_FAIL("INF result was not detected: " << x << " returned");
    } catch (Error&) {
        // as expected. Do nothing.

        // Note: we're expecting an Error we threw, not just any
        // exception.  If something else is thrown, then there's
        // another problem and the test must fail.
    }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
