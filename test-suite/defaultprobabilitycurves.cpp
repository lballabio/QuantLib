/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

#include "defaultprobabilitycurves.hpp"
#include "utilities.hpp"
#include <ql/termstructures/credit/piecewisedefaultcurve.hpp>
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void DefaultProbabilityCurveTest::testDefaultProbability() {

    BOOST_TEST_MESSAGE("Testing default-probability structure...");

    Real hazardRate = 0.0100;
    Handle<Quote> hazardRateQuote = Handle<Quote>(
                boost::shared_ptr<Quote>(new SimpleQuote(hazardRate)));
    DayCounter dayCounter = Actual360();
    Calendar calendar = TARGET();
    Size n = 20;

    double tolerance = 1.0e-10;
    Date today = Settings::instance().evaluationDate();
    Date startDate = today;
    Date endDate = startDate;

    FlatHazardRate flatHazardRate(startDate, hazardRateQuote, dayCounter);

    for(Size i=0; i<n; i++){
        startDate = endDate;
        endDate = calendar.advance(endDate, 1, Years);

        Probability pStart = flatHazardRate.defaultProbability(startDate);
        Probability pEnd = flatHazardRate.defaultProbability(endDate);

        Probability pBetweenComputed =
            flatHazardRate.defaultProbability(startDate, endDate);

        Probability pBetween = pEnd - pStart;

        if (std::fabs(pBetween - pBetweenComputed) > tolerance)
            BOOST_ERROR(
                "Failed to reproduce probability(d1, d2) "
                << "for default probability structure\n"
                << std::setprecision(12)
                << "    calculated probability: " << pBetweenComputed << "\n"
                << "    expected probability:   " << pBetween);

        Time t2 = dayCounter.yearFraction(today, endDate);
        Probability timeProbability = flatHazardRate.defaultProbability(t2);
        Probability dateProbability =
            flatHazardRate.defaultProbability(endDate);

        if (std::fabs(timeProbability - dateProbability) > tolerance)
            BOOST_ERROR(
                "single-time probability and single-date probability do not match\n"
                << std::setprecision(10)
                << "    time probability: " << timeProbability << "\n"
                << "    date probability: " << dateProbability);

        Time t1 = dayCounter.yearFraction(today, startDate);
        timeProbability = flatHazardRate.defaultProbability(t1, t2);
        dateProbability = flatHazardRate.defaultProbability(startDate, endDate);

        if (std::fabs(timeProbability - dateProbability) > tolerance)
            BOOST_ERROR(
                "double-time probability and double-date probability do not match\n"
                << std::setprecision(10)
                << "    time probability: " << timeProbability << "\n"
                << "    date probability: " << dateProbability);
    }
}


void DefaultProbabilityCurveTest::testFlatHazardRate() {

    BOOST_TEST_MESSAGE("Testing flat hazard rate...");

    Real hazardRate = 0.0100;
    Handle<Quote> hazardRateQuote = Handle<Quote>(
                boost::shared_ptr<Quote>(new SimpleQuote(hazardRate)));
    DayCounter dayCounter = Actual360();
    Calendar calendar = TARGET();
    Size n = 20;

    double tolerance = 1.0e-10;
    Date today = Settings::instance().evaluationDate();
    Date startDate = today;
    Date endDate = startDate;

    FlatHazardRate flatHazardRate(today, hazardRateQuote, dayCounter);

    for(Size i=0; i<n; i++){
        endDate = calendar.advance(endDate, 1, Years);
        Time t = dayCounter.yearFraction(startDate, endDate);
        Probability probability = 1.0 - std::exp(-hazardRate * t);
        Probability computedProbability = flatHazardRate.defaultProbability(t);

        if (std::fabs(probability - computedProbability) > tolerance)
            BOOST_ERROR(
                "Failed to reproduce probability for flat hazard rate\n"
                << std::setprecision(10)
                << "    calculated probability: " << computedProbability << "\n"
                << "    expected probability:   " << probability);
    }
}


namespace {

    template <class T, class I>
    void testBootstrapFromSpread() {

        Calendar calendar = TARGET();

        Date today = Settings::instance().evaluationDate();

        Integer settlementDays = 1;

        std::vector<Real> quote;
        quote.push_back(0.005);
        quote.push_back(0.006);
        quote.push_back(0.007);
        quote.push_back(0.009);

        std::vector<Integer> n;
        n.push_back(1);
        n.push_back(2);
        n.push_back(3);
        n.push_back(5);

        Frequency frequency = Quarterly;
        BusinessDayConvention convention = Following;
        DateGeneration::Rule rule = DateGeneration::TwentiethIMM;
        DayCounter dayCounter = Thirty360();
        Real recoveryRate = 0.4;

        RelinkableHandle<YieldTermStructure> discountCurve;
        discountCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

        std::vector<boost::shared_ptr<DefaultProbabilityHelper> > helpers;

        for(Size i=0; i<n.size(); i++)
            helpers.push_back(
                boost::shared_ptr<DefaultProbabilityHelper>(
                    new SpreadCdsHelper(quote[i], Period(n[i], Years),
                                        settlementDays, calendar,
                                        frequency, convention, rule,
                                        dayCounter, recoveryRate,
                                        discountCurve)));

        RelinkableHandle<DefaultProbabilityTermStructure> piecewiseCurve;
        piecewiseCurve.linkTo(
            boost::shared_ptr<DefaultProbabilityTermStructure>(
                new PiecewiseDefaultCurve<T,I>(today, helpers,
                                               Thirty360())));

        Real notional = 1.0;
        double tolerance = 1.0e-6;

        // ensure apple-to-apple comparison
        SavedSettings backup;
        Settings::instance().includeTodaysCashFlows() = true;

        for (Size i=0; i<n.size(); i++) {
            Date protectionStart = today + settlementDays;
            Date startDate = calendar.adjust(protectionStart, convention);
            Date endDate = today + n[i]*Years;

            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              convention, Unadjusted, rule, false);

            CreditDefaultSwap cds(Protection::Buyer, notional, quote[i],
                                  schedule, convention, dayCounter,
                                  true, true, protectionStart);
            cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                           new MidPointCdsEngine(piecewiseCurve, recoveryRate,
                                                 discountCurve)));

            // test
            Rate inputRate = quote[i];
            Rate computedRate = cds.fairSpread();
            if (std::fabs(inputRate - computedRate) > tolerance)
                BOOST_ERROR(
                    "\nFailed to reproduce fair spread for " << n[i] <<
                    "Y credit-default swaps\n"
                    << std::setprecision(10)
                    << "    computed rate: " << io::rate(computedRate) << "\n"
                    << "    input rate:    " << io::rate(inputRate));
        }
    }


    template <class T, class I>
    void testBootstrapFromUpfront() {

        Calendar calendar = TARGET();

        Date today = Settings::instance().evaluationDate();

        Integer settlementDays = 0;

        std::vector<Real> quote;
        quote.push_back(0.01);
        quote.push_back(0.02);
        quote.push_back(0.04);
        quote.push_back(0.06);

        std::vector<Integer> n;
        n.push_back(2);
        n.push_back(3);
        n.push_back(5);
        n.push_back(7);

        Rate fixedRate = 0.05;
        Frequency frequency = Quarterly;
        BusinessDayConvention convention = Following;
        DateGeneration::Rule rule = DateGeneration::TwentiethIMM;
        DayCounter dayCounter = Thirty360();
        Real recoveryRate = 0.4;

        RelinkableHandle<YieldTermStructure> discountCurve;
        discountCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

        std::vector<boost::shared_ptr<DefaultProbabilityHelper> > helpers;

        for(Size i=0; i<n.size(); i++)
            helpers.push_back(
                boost::shared_ptr<DefaultProbabilityHelper>(
                    new UpfrontCdsHelper(quote[i], fixedRate,
                                         Period(n[i], Years),
                                         settlementDays, calendar,
                                         frequency, convention, rule,
                                         dayCounter, recoveryRate,
                                         discountCurve)));

        RelinkableHandle<DefaultProbabilityTermStructure> piecewiseCurve;
        piecewiseCurve.linkTo(
            boost::shared_ptr<DefaultProbabilityTermStructure>(
                new PiecewiseDefaultCurve<T,I>(today, helpers,
                                               Thirty360())));

        Real notional = 1.0;
        double tolerance = 1.0e-6;

        SavedSettings backup;
        // ensure apple-to-apple comparison
        Settings::instance().includeTodaysCashFlows() = true;

        for (Size i=0; i<n.size(); i++) {
            Date protectionStart = today + settlementDays;
            Date startDate = calendar.adjust(protectionStart, convention);
            Date endDate = today + n[i]*Years;

            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              convention, Unadjusted, rule, false);

            CreditDefaultSwap cds(Protection::Buyer, notional,
                                  quote[i], fixedRate,
                                  schedule, convention, dayCounter,
                                  true, true, protectionStart);
            cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                           new MidPointCdsEngine(piecewiseCurve, recoveryRate,
                                                 discountCurve, true)));

            // test
            Rate inputUpfront = quote[i];
            Rate computedUpfront = cds.fairUpfront();
            if (std::fabs(inputUpfront - computedUpfront) > tolerance)
                BOOST_ERROR(
                    "\nFailed to reproduce fair upfront for " << n[i] <<
                    "Y credit-default swaps\n"
                    << std::setprecision(10)
                    << "    computed: " << io::rate(computedUpfront) << "\n"
                    << "    expected: " << io::rate(inputUpfront));
        }
    }

}

void DefaultProbabilityCurveTest::testFlatHazardConsistency() {
    BOOST_TEST_MESSAGE("Testing piecewise-flat hazard-rate consistency...");
    testBootstrapFromSpread<HazardRate,BackwardFlat>();
    testBootstrapFromUpfront<HazardRate,BackwardFlat>();
}

void DefaultProbabilityCurveTest::testFlatDensityConsistency() {
    BOOST_TEST_MESSAGE("Testing piecewise-flat default-density consistency...");
    testBootstrapFromSpread<DefaultDensity,BackwardFlat>();
    testBootstrapFromUpfront<DefaultDensity,BackwardFlat>();
}

void DefaultProbabilityCurveTest::testLinearDensityConsistency() {
    BOOST_TEST_MESSAGE("Testing piecewise-linear default-density consistency...");
    testBootstrapFromSpread<DefaultDensity,Linear>();
    testBootstrapFromUpfront<DefaultDensity,Linear>();
}

void DefaultProbabilityCurveTest::testLogLinearSurvivalConsistency() {
    BOOST_TEST_MESSAGE("Testing log-linear survival-probability consistency...");
    testBootstrapFromSpread<SurvivalProbability,LogLinear>();
    testBootstrapFromUpfront<SurvivalProbability,LogLinear>();
}

void DefaultProbabilityCurveTest::testSingleInstrumentBootstrap() {
    BOOST_TEST_MESSAGE("Testing single-instrument curve bootstrap...");

    Calendar calendar = TARGET();

    Date today = Settings::instance().evaluationDate();

    Integer settlementDays = 0;

    Real quote = 0.005;
    Period tenor = 2*Years;

    Frequency frequency = Quarterly;
    BusinessDayConvention convention = Following;
    DateGeneration::Rule rule = DateGeneration::TwentiethIMM;
    DayCounter dayCounter = Thirty360();
    Real recoveryRate = 0.4;

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(boost::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

    std::vector<boost::shared_ptr<DefaultProbabilityHelper> > helpers(1);

    helpers[0] = boost::shared_ptr<DefaultProbabilityHelper>(
                        new SpreadCdsHelper(quote, tenor,
                                            settlementDays, calendar,
                                            frequency, convention, rule,
                                            dayCounter, recoveryRate,
                                            discountCurve));

    PiecewiseDefaultCurve<HazardRate,BackwardFlat> defaultCurve(today, helpers,
                                                                dayCounter);
    defaultCurve.recalculate();
}

void DefaultProbabilityCurveTest::testUpfrontBootstrap() {
    BOOST_TEST_MESSAGE("Testing bootstrap on upfront quotes...");

    SavedSettings backup;
    // not taken into account, this would prevent the upfront from being used
    Settings::instance().includeTodaysCashFlows() = false;

    testBootstrapFromUpfront<HazardRate,BackwardFlat>();

    // also ensure that we didn't override the flag permanently
    boost::optional<bool> flag = Settings::instance().includeTodaysCashFlows();
    if (flag != false)
        BOOST_ERROR("Cash-flow settings improperly modified");
}


test_suite* DefaultProbabilityCurveTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Default-probability curve tests");
    suite->add(QUANTLIB_TEST_CASE(
                       &DefaultProbabilityCurveTest::testDefaultProbability));
    suite->add(QUANTLIB_TEST_CASE(
                           &DefaultProbabilityCurveTest::testFlatHazardRate));
    suite->add(QUANTLIB_TEST_CASE(
                    &DefaultProbabilityCurveTest::testFlatHazardConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                   &DefaultProbabilityCurveTest::testFlatDensityConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                 &DefaultProbabilityCurveTest::testLinearDensityConsistency));
    suite->add(QUANTLIB_TEST_CASE(
             &DefaultProbabilityCurveTest::testLogLinearSurvivalConsistency));
    suite->add(QUANTLIB_TEST_CASE(
                &DefaultProbabilityCurveTest::testSingleInstrumentBootstrap));
    suite->add(QUANTLIB_TEST_CASE(
                         &DefaultProbabilityCurveTest::testUpfrontBootstrap));
    return suite;
}
