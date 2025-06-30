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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/interpolations/loginterpolation.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/credit/piecewisedefaultcurve.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <iomanip>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using std::map;
using std::vector;
using std::string;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(DefaultProbabilityCurveTests)

BOOST_AUTO_TEST_CASE(testDefaultProbability) {

    BOOST_TEST_MESSAGE("Testing default-probability structure...");

    Real hazardRate = 0.0100;
    Handle<Quote> hazardRateQuote = Handle<Quote>(
                ext::shared_ptr<Quote>(new SimpleQuote(hazardRate)));
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

BOOST_AUTO_TEST_CASE(testFlatHazardRate) {

    BOOST_TEST_MESSAGE("Testing flat hazard rate...");

    Real hazardRate = 0.0100;
    Handle<Quote> hazardRateQuote = Handle<Quote>(
                ext::shared_ptr<Quote>(new SimpleQuote(hazardRate)));
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


template <class T, class I>
void testBootstrapFromSpread() {

    Calendar calendar = TARGET();

    Date today = Settings::instance().evaluationDate();

    Integer settlementDays = 1;

    std::vector<Real> quote = {0.005, 0.006, 0.007, 0.009};
    std::vector<Integer> n = {1, 2, 3, 5};

    Frequency frequency = Quarterly;
    BusinessDayConvention convention = Following;
    DateGeneration::Rule rule = DateGeneration::TwentiethIMM;
    DayCounter dayCounter = Thirty360(Thirty360::BondBasis);
    Real recoveryRate = 0.4;

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

    std::vector<ext::shared_ptr<DefaultProbabilityHelper> > helpers;

    helpers.reserve(n.size());
    for(Size i=0; i<n.size(); i++)
        helpers.push_back(
                ext::shared_ptr<DefaultProbabilityHelper>(
                    new SpreadCdsHelper(quote[i], Period(n[i], Years),
                                        settlementDays, calendar,
                                        frequency, convention, rule,
                                        dayCounter, recoveryRate,
                                        discountCurve)));

    RelinkableHandle<DefaultProbabilityTermStructure> piecewiseCurve;
    piecewiseCurve.linkTo(
            ext::shared_ptr<DefaultProbabilityTermStructure>(
                new PiecewiseDefaultCurve<T,I>(today, helpers,
                                               Thirty360(Thirty360::BondBasis))));

    Real notional = 1.0;
    double tolerance = 1.0e-6;

    {
        SavedSettings restore;
        // ensure apple-to-apple comparison
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
            cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                           new MidPointCdsEngine(piecewiseCurve, recoveryRate,
                                                 discountCurve)));

            // test
            Rate inputRate = quote[i];
            Rate computedRate = cds.fairSpread();
            if (std::fabs(inputRate - computedRate) > tolerance)
                BOOST_ERROR("\nFailed to reproduce fair spread for " << n[i] <<
                            "Y credit-default swaps\n"
                            << std::setprecision(10)
                            << "    computed rate: " << io::rate(computedRate) << "\n"
                            << "    input rate:    " << io::rate(inputRate));
        }
    }
}


template <class T, class I>
void testBootstrapFromUpfront() {

    Calendar calendar = TARGET();

    Date today = Settings::instance().evaluationDate();

    Integer settlementDays = 1;

    std::vector<Real> quote = {0.01, 0.02, 0.04, 0.06};
    std::vector<Integer> n = {2, 3, 5, 7};

    Rate fixedRate = 0.05;
    Frequency frequency = Quarterly;
    BusinessDayConvention convention = ModifiedFollowing;
    DateGeneration::Rule rule = DateGeneration::CDS;
    DayCounter dayCounter = Actual360();
    Real recoveryRate = 0.4;
    Integer upfrontSettlementDays = 3;

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

    std::vector<ext::shared_ptr<DefaultProbabilityHelper> > helpers;

    helpers.reserve(n.size());
    for(Size i=0; i<n.size(); i++)
        helpers.push_back(
                ext::shared_ptr<DefaultProbabilityHelper>(
                    new UpfrontCdsHelper(quote[i], fixedRate,
                                         Period(n[i], Years),
                                         settlementDays, calendar,
                                         frequency, convention, rule,
                                         dayCounter, recoveryRate,
                                         discountCurve,
                                         upfrontSettlementDays, 
                                         true, true, Date(), Actual360(true))));

    RelinkableHandle<DefaultProbabilityTermStructure> piecewiseCurve;
    piecewiseCurve.linkTo(
            ext::shared_ptr<DefaultProbabilityTermStructure>(
                new PiecewiseDefaultCurve<T,I>(today, helpers,
                                               Thirty360(Thirty360::BondBasis))));

    Real notional = 1.0;
    double tolerance = 1.0e-6;

    {
        SavedSettings backup;
        // ensure apple-to-apple comparison
        Settings::instance().includeTodaysCashFlows() = true;

        for (Size i=0; i<n.size(); i++) {
            Date protectionStart = today + settlementDays;
            Date startDate = protectionStart;
            Date endDate = cdsMaturity(today, n[i] * Years, rule);
            Date upfrontDate = calendar.advance(today,
                                                upfrontSettlementDays,
                                                Days,
                                                convention);

            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              convention, Unadjusted, rule, false);

            CreditDefaultSwap cds(Protection::Buyer, notional,
                                  quote[i], fixedRate,
                                  schedule, convention, dayCounter,
                                  true, true, protectionStart,
                                  upfrontDate,
                                  ext::shared_ptr<Claim>(),
                                  Actual360(true),
                                  true, today);
            cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                           new MidPointCdsEngine(piecewiseCurve, recoveryRate,
                                                 discountCurve, true)));

            // test
            Rate inputUpfront = quote[i];
            Rate computedUpfront = cds.fairUpfront();
            if (std::fabs(inputUpfront - computedUpfront) > tolerance)
                BOOST_ERROR("\nFailed to reproduce fair upfront for " << n[i] <<
                            "Y credit-default swaps\n"
                            << std::setprecision(10)
                            << "    computed: " << io::rate(computedUpfront) << "\n"
                            << "    expected: " << io::rate(inputUpfront));
        }
    }
}


BOOST_AUTO_TEST_CASE(testFlatHazardConsistency) {
    BOOST_TEST_MESSAGE("Testing piecewise-flat hazard-rate consistency...");
    testBootstrapFromSpread<HazardRate,BackwardFlat>();
    testBootstrapFromUpfront<HazardRate,BackwardFlat>();
}

BOOST_AUTO_TEST_CASE(testFlatDensityConsistency) {
    BOOST_TEST_MESSAGE("Testing piecewise-flat default-density consistency...");
    testBootstrapFromSpread<DefaultDensity,BackwardFlat>();
    testBootstrapFromUpfront<DefaultDensity,BackwardFlat>();
}

BOOST_AUTO_TEST_CASE(testLinearDensityConsistency) {
    BOOST_TEST_MESSAGE("Testing piecewise-linear default-density consistency...");
    testBootstrapFromSpread<DefaultDensity,Linear>();
    testBootstrapFromUpfront<DefaultDensity,Linear>();
}

BOOST_AUTO_TEST_CASE(testLogLinearSurvivalConsistency) {
    BOOST_TEST_MESSAGE("Testing log-linear survival-probability consistency...");
    testBootstrapFromSpread<SurvivalProbability,LogLinear>();
    testBootstrapFromUpfront<SurvivalProbability,LogLinear>();
}

BOOST_AUTO_TEST_CASE(testSingleInstrumentBootstrap) {
    BOOST_TEST_MESSAGE("Testing single-instrument curve bootstrap...");

    Calendar calendar = TARGET();

    Date today = Settings::instance().evaluationDate();

    Integer settlementDays = 0;

    Real quote = 0.005;
    Period tenor = 2*Years;

    Frequency frequency = Quarterly;
    BusinessDayConvention convention = Following;
    DateGeneration::Rule rule = DateGeneration::TwentiethIMM;
    DayCounter dayCounter = Thirty360(Thirty360::BondBasis);
    Real recoveryRate = 0.4;

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                                    new FlatForward(today,0.06,Actual360())));

    std::vector<ext::shared_ptr<DefaultProbabilityHelper> > helpers(1);

    helpers[0] = ext::shared_ptr<DefaultProbabilityHelper>(
                        new SpreadCdsHelper(quote, tenor,
                                            settlementDays, calendar,
                                            frequency, convention, rule,
                                            dayCounter, recoveryRate,
                                            discountCurve));

    PiecewiseDefaultCurve<HazardRate,BackwardFlat> defaultCurve(today, helpers,
                                                                dayCounter);
    defaultCurve.recalculate();
}

BOOST_AUTO_TEST_CASE(testUpfrontBootstrap) {
    BOOST_TEST_MESSAGE("Testing bootstrap on upfront quotes...");

    // Setting this to false would prevent the upfront from being used.
    // By checking that the bootstrap works, we indirectly check that
    // UpfrontCdsHelper::impliedQuote() overrides it.
    Settings::instance().includeTodaysCashFlows() = false;

    testBootstrapFromUpfront<HazardRate,BackwardFlat>();

    // This checks that UpfrontCdsHelper::impliedQuote() didn't
    // override the flag permanently; after the bootstrap, it should
    // go back to its previous value.
    ext::optional<bool> flag = Settings::instance().includeTodaysCashFlows();
    if (flag != false)
        BOOST_ERROR("Cash-flow settings improperly modified");
}

/* This test attempts to build a default curve from CDS spreads as of 1 Apr 2020. The spreads are real and from a 
   distressed reference entity with an inverted CDS spread curve. Using the default IterativeBootstrap with no 
   retries, the default curve building fails. Allowing retries, it expands the min survival probability bounds but 
   still fails. We set dontThrow to true in IterativeBootstrap to use a fall back curve.
*/

BOOST_AUTO_TEST_CASE(testIterativeBootstrapRetries) {

    BOOST_TEST_MESSAGE("Testing iterative bootstrap with retries...");

    Date asof(1, Apr, 2020);
    Settings::instance().evaluationDate() = asof;
    Actual365Fixed tsDayCounter;

    // USD discount curve built out of FedFunds OIS swaps.
    vector<Date> usdCurveDates = {
        Date(1, Apr, 2020),
        Date(2, Apr, 2020),
        Date(14, Apr, 2020),
        Date(21, Apr, 2020),
        Date(28, Apr, 2020),
        Date(6, May, 2020),
        Date(5, Jun, 2020),
        Date(7, Jul, 2020),
        Date(5, Aug, 2020),
        Date(8, Sep, 2020),
        Date(7, Oct, 2020),
        Date(5, Nov, 2020),
        Date(7, Dec, 2020),
        Date(6, Jan, 2021),
        Date(5, Feb, 2021),
        Date(5, Mar, 2021),
        Date(7, Apr, 2021),
        Date(4, Apr, 2022),
        Date(3, Apr, 2023),
        Date(3, Apr, 2024),
        Date(3, Apr, 2025),
        Date(5, Apr, 2027),
        Date(3, Apr, 2030),
        Date(3, Apr, 2035),
        Date(3, Apr, 2040),
        Date(4, Apr, 2050)
    };

    vector<DiscountFactor> usdCurveDfs = {
        1.000000000,
        0.999955835,
        0.999931070,
        0.999914629,
        0.999902799,
        0.999887990,
        0.999825782,
        0.999764392,
        0.999709076,
        0.999647785,
        0.999594638,
        0.999536198,
        0.999483093,
        0.999419291,
        0.999379417,
        0.999324981,
        0.999262356,
        0.999575101,
        0.996135441,
        0.995228348,
        0.989366687,
        0.979271200,
        0.961150726,
        0.926265361,
        0.891640651,
        0.839314063
    };

    Handle<YieldTermStructure> usdYts(ext::make_shared<InterpolatedDiscountCurve<LogLinear> >(
        usdCurveDates, usdCurveDfs, tsDayCounter));

    // CDS spreads
    map<Period, Rate> cdsSpreads = {
        {6 * Months, 2.957980250},
        {1 * Years, 3.076933100},
        {2 * Years, 2.944524520},
        {3 * Years, 2.844498960},
        {4 * Years, 2.769234420},
        {5 * Years, 2.713474100}
    };
    Real recoveryRate = 0.035;

    // Conventions
    Integer settlementDays = 1;
    WeekendsOnly calendar;
    Frequency frequency = Quarterly;
    BusinessDayConvention paymentConvention = Following;
    DateGeneration::Rule rule = DateGeneration::CDS2015;
    Actual360 dayCounter;
    Actual360 lastPeriodDayCounter(true);

    // Create the CDS spread helpers.
    vector<ext::shared_ptr<DefaultProbabilityHelper> > instruments;
    instruments.reserve(cdsSpreads.size());
    for (auto & cdsSpread : cdsSpreads) {
        instruments.push_back(ext::make_shared<SpreadCdsHelper>(
            cdsSpread.second, cdsSpread.first, settlementDays, calendar,
                                frequency, paymentConvention, rule, dayCounter, recoveryRate, usdYts, true, true, Date(),
                                lastPeriodDayCounter));
    }

    // Create the default curve with the default IterativeBootstrap.
    typedef PiecewiseDefaultCurve<SurvivalProbability, LogLinear, IterativeBootstrap> SPCurve;
    ext::shared_ptr<DefaultProbabilityTermStructure> dpts = ext::make_shared<SPCurve>(asof, instruments, tsDayCounter);

    // Check that the default curve throws by requesting a default probability.
    Date testDate(21, Dec, 2020);
    BOOST_CHECK_EXCEPTION(dpts->survivalProbability(testDate), Error,
        ExpectedErrorMessage("1st iteration: failed at 1st alive instrument"));

    // Create the default curve with an IterativeBootstrap allowing for 4 retries.
    // Use a maxFactor value of 1.0 so that we still use the previous survival probability at each pillar. In other
    // words, the survival probability cannot increase with time so best max at current pillar is the previous 
    // pillar's value - there is no point increasing it on a retry.
    IterativeBootstrap<SPCurve> ib(Null<Real>(), Null<Real>(), Null<Real>(), 5, 1.0, 10.0);
    dpts = ext::make_shared<SPCurve>(asof, instruments, tsDayCounter, ib);

    // Check that the default curve still throws. It throws at the third pillar because the survival probability is 
    // too low at the second pillar.
    BOOST_CHECK_EXCEPTION(dpts->survivalProbability(testDate), Error,
        ExpectedErrorMessage("1st iteration: failed at 3rd alive instrument"));

    // Create the default curve with an IterativeBootstrap that allows for 4 retries and does not throw.
    IterativeBootstrap<SPCurve> ibNoThrow(Null<Real>(), Null<Real>(), Null<Real>(), 5, 1.0, 10.0, true, 2);
    dpts = ext::make_shared<SPCurve>(asof, instruments, tsDayCounter, ibNoThrow);
    BOOST_CHECK_NO_THROW(dpts->survivalProbability(testDate));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
