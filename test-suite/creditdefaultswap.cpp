/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 StatPro Italia srl

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
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/instruments/makecds.hpp>
#include <ql/pricingengines/credit/midpointcdsengine.hpp>
#include <ql/pricingengines/credit/integralcdsengine.hpp>
#include <ql/pricingengines/credit/isdacdsengine.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/credit/interpolatedhazardratecurve.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/discountcurve.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/interpolations/backwardflatinterpolation.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/calendars/unitedstates.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/optional.hpp>
#include <map>
#include <iomanip>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using std::map;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CreditDefaultSwapTests)

BOOST_AUTO_TEST_CASE(testCachedValue) {

    BOOST_TEST_MESSAGE("Testing credit-default swap against cached values...");

    // Initialize curves
    Settings::instance().evaluationDate() = Date(9,June,2006);
    Date today = Settings::instance().evaluationDate();
    Calendar calendar = TARGET();

    Handle<Quote> hazardRate = Handle<Quote>(
                ext::shared_ptr<Quote>(new SimpleQuote(0.01234)));
    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(
        ext::shared_ptr<DefaultProbabilityTermStructure>(
                   new FlatHazardRate(0, calendar, hazardRate, Actual360())));

    RelinkableHandle<YieldTermStructure> discountCurve;

    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.06,Actual360())));

    // Build the schedule
    Date issueDate = calendar.advance(today, -1, Years);
    Date maturity = calendar.advance(issueDate, 10, Years);
    Frequency frequency = Semiannual;
    BusinessDayConvention convention = ModifiedFollowing;

    Schedule schedule(issueDate, maturity, Period(frequency), calendar,
                      convention, convention, DateGeneration::Forward, false);

    // Build the CDS
    Rate fixedRate = 0.0120;
    DayCounter dayCount = Actual360();
    Real notional = 10000.0;
    Real recoveryRate = 0.4;

    CreditDefaultSwap cds(Protection::Seller, notional, fixedRate,
                          schedule, convention, dayCount, true, true);
    cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
         new MidPointCdsEngine(probabilityCurve,recoveryRate,discountCurve)));

    Real npv = 295.0153398;
    Rate fairRate = 0.007517539081;

    Real calculatedNpv = cds.NPV();
    Rate calculatedFairRate = cds.fairSpread();
    Real tolerance = 1.0e-7;

    if (std::fabs(calculatedNpv - npv) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce NPV with mid-point engine\n"
            << std::setprecision(10)
            << "    calculated NPV: " << calculatedNpv << "\n"
            << "    expected NPV:   " << npv);

    if (std::fabs(calculatedFairRate - fairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce fair rate with mid-point engine\n"
            << std::setprecision(10)
            << "    calculated fair rate: " << calculatedFairRate << "\n"
            << "    expected fair rate:   " << fairRate);

    cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                          new IntegralCdsEngine(1*Days,probabilityCurve,
                                                recoveryRate,discountCurve)));

    calculatedNpv = cds.NPV();
    calculatedFairRate = cds.fairSpread();
    tolerance = 1.0e-5;

    if (std::fabs(calculatedNpv - npv) > notional*tolerance*10)
        BOOST_ERROR(
            "Failed to reproduce NPV with integral engine "
            "(step = 1 day)\n"
            << std::setprecision(10)
            << "    calculated NPV: " << calculatedNpv << "\n"
            << "    expected NPV:   " << npv);

    if (std::fabs(calculatedFairRate - fairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce fair rate with integral engine "
            "(step = 1 day)\n"
            << std::setprecision(10)
            << "    calculated fair rate: " << calculatedFairRate << "\n"
            << "    expected fair rate:   " << fairRate);

    cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                          new IntegralCdsEngine(1*Weeks,probabilityCurve,
                                                recoveryRate,discountCurve)));

    calculatedNpv = cds.NPV();
    calculatedFairRate = cds.fairSpread();
    tolerance = 1.0e-5;

    if (std::fabs(calculatedNpv - npv) > notional*tolerance*10)
        BOOST_ERROR(
            "Failed to reproduce NPV with integral engine "
            "(step = 1 week)\n"
            << std::setprecision(10)
            << "    calculated NPV: " << calculatedNpv << "\n"
            << "    expected NPV:   " << npv);

    if (std::fabs(calculatedFairRate - fairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce fair rate with integral engine "
            "(step = 1 week)\n"
            << std::setprecision(10)
            << "    calculated fair rate: " << calculatedFairRate << "\n"
            << "    expected fair rate:   " << fairRate);
}

BOOST_AUTO_TEST_CASE(testCachedMarketValue) {

    BOOST_TEST_MESSAGE(
        "Testing credit-default swap against cached market values...");

    Settings::instance().evaluationDate() = Date(9,June,2006);
    Date evalDate = Settings::instance().evaluationDate();
    Calendar calendar = UnitedStates(UnitedStates::GovernmentBond);

    std::vector<Date> discountDates = {
        evalDate,
        calendar.advance(evalDate, 1, Weeks,  ModifiedFollowing),
        calendar.advance(evalDate, 1, Months, ModifiedFollowing),
        calendar.advance(evalDate, 2, Months, ModifiedFollowing),
        calendar.advance(evalDate, 3, Months, ModifiedFollowing),
        calendar.advance(evalDate, 6, Months, ModifiedFollowing),
        calendar.advance(evalDate,12, Months, ModifiedFollowing),
        calendar.advance(evalDate, 2, Years, ModifiedFollowing),
        calendar.advance(evalDate, 3, Years, ModifiedFollowing),
        calendar.advance(evalDate, 4, Years, ModifiedFollowing),
        calendar.advance(evalDate, 5, Years, ModifiedFollowing),
        calendar.advance(evalDate, 6, Years, ModifiedFollowing),
        calendar.advance(evalDate, 7, Years, ModifiedFollowing),
        calendar.advance(evalDate, 8, Years, ModifiedFollowing),
        calendar.advance(evalDate, 9, Years, ModifiedFollowing),
        calendar.advance(evalDate,10, Years, ModifiedFollowing),
        calendar.advance(evalDate,15, Years, ModifiedFollowing)
    };
    
    std::vector<DiscountFactor> dfs = {
        1.0,
        0.9990151375768731,
        0.99570502636871183,
        0.99118260474528685,
        0.98661167950906203,
        0.9732592953359388,
        0.94724424481038083,
        0.89844996737120875,
        0.85216647839921411,
        0.80775477692556874,
        0.76517289234200347,
        0.72401019553182933,
        0.68503909569219212,
        0.64797499814013748,
        0.61263171936255534,
        0.5791942350748791,
        0.43518868769953606
    };

    const DayCounter& curveDayCounter=Actual360();

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
        ext::shared_ptr<YieldTermStructure>(
            new DiscountCurve(discountDates, dfs, curveDayCounter)));

    DayCounter dayCounter = Thirty360(Thirty360::BondBasis);
    std::vector<Date> dates = {
        evalDate,
        calendar.advance(evalDate, 6, Months, ModifiedFollowing),
        calendar.advance(evalDate, 1, Years, ModifiedFollowing),
        calendar.advance(evalDate, 2, Years, ModifiedFollowing),
        calendar.advance(evalDate, 3, Years, ModifiedFollowing),
        calendar.advance(evalDate, 4, Years, ModifiedFollowing),
        calendar.advance(evalDate, 5, Years, ModifiedFollowing),
        calendar.advance(evalDate, 7, Years, ModifiedFollowing),
        calendar.advance(evalDate,10, Years, ModifiedFollowing)
    };

    std::vector<Probability> defaultProbabilities = {
        0.0000,
        0.0047,
        0.0093,
        0.0286,
        0.0619,
        0.0953,
        0.1508,
        0.2288,
        0.3666
    };

    std::vector<Real> hazardRates;
    hazardRates.push_back(0.0);
    for (Size i=1; i<dates.size(); ++i) {
        Time t1 = dayCounter.yearFraction(dates[0], dates[i-1]);
        Time t2 = dayCounter.yearFraction(dates[0], dates[i]);
        Probability S1 = 1.0 - defaultProbabilities[i-1];
        Probability S2 = 1.0 - defaultProbabilities[i];
        hazardRates.push_back(std::log(S1/S2)/(t2-t1));
    }

    RelinkableHandle<DefaultProbabilityTermStructure> piecewiseFlatHazardRate;
    piecewiseFlatHazardRate.linkTo(
        ext::shared_ptr<DefaultProbabilityTermStructure>(
               new InterpolatedHazardRateCurve<BackwardFlat>(dates,
                                                             hazardRates,
                                                             Thirty360(Thirty360::BondBasis))));

    // Testing credit default swap

    // Build the schedule
    Date issueDate(20, March, 2006);
    Date maturity(20, June, 2013);
    Frequency cdsFrequency = Semiannual;
    BusinessDayConvention cdsConvention = ModifiedFollowing;

    Schedule schedule(issueDate, maturity, Period(cdsFrequency), calendar,
                      cdsConvention, cdsConvention,
                      DateGeneration::Forward, false);

    // Build the CDS
    Real recoveryRate = 0.25;
    Rate fixedRate=0.0224;
    DayCounter dayCount=Actual360();
    Real cdsNotional=100.0;

    CreditDefaultSwap cds(Protection::Seller, cdsNotional, fixedRate,
                          schedule, cdsConvention, dayCount, true, true);
    cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                          new MidPointCdsEngine(piecewiseFlatHazardRate,
                                                recoveryRate,discountCurve)));

    Real calculatedNpv = cds.NPV();
    Real calculatedFairRate = cds.fairSpread();

    double npv = -1.364048777;        // from Bloomberg we have 98.15598868 - 100.00;
    double fairRate =  0.0248429452; // from Bloomberg we have 0.0258378;

    Real tolerance = 1e-9;

    if (std::fabs(npv - calculatedNpv) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce the npv for the given credit-default swap\n"
            << std::setprecision(10)
            << "    computed NPV:  " << calculatedNpv << "\n"
            << "    Given NPV:     " << npv);

    if (std::fabs(fairRate - calculatedFairRate) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce the fair rate for the given credit-default swap\n"
            << std::setprecision(10)
            << "    computed fair rate:  " << calculatedFairRate << "\n"
            << "    Given fair rate:     " << fairRate);
}

BOOST_AUTO_TEST_CASE(testImpliedHazardRate) {

    BOOST_TEST_MESSAGE("Testing implied hazard-rate for credit-default swaps...");

    // Initialize curves
    Calendar calendar = TARGET();
    Date today = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today;

    Rate h1 = 0.30, h2 = 0.40;
    DayCounter dayCounter = Actual365Fixed();

    std::vector<Date> dates(3);
    std::vector<Real> hazardRates(3);
    dates[0] = today;
    hazardRates[0] = h1;

    dates[1] = today + 5*Years;
    hazardRates[1] = h1;

    dates[2] = today + 10*Years;
    hazardRates[2] = h2;

    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(ext::shared_ptr<DefaultProbabilityTermStructure>(
                    new InterpolatedHazardRateCurve<BackwardFlat>(dates,
                                                                  hazardRates,
                                                                  dayCounter)));

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.03,Actual360())));


    Frequency frequency = Semiannual;
    BusinessDayConvention convention = ModifiedFollowing;

    Date issueDate = calendar.advance(today, -6, Months);
    Rate fixedRate = 0.0120;
    DayCounter cdsDayCount = Actual360();
    Real notional = 10000.0;
    Real recoveryRate = 0.4;

    Rate latestRate = Null<Rate>();
    for (Integer n=6; n<=10; ++n) {

        Date maturity = calendar.advance(issueDate, n, Years);
        Schedule schedule(issueDate, maturity, Period(frequency), calendar,
                          convention, convention,
                          DateGeneration::Forward, false);

        CreditDefaultSwap cds(Protection::Seller, notional, fixedRate,
                              schedule, convention, cdsDayCount,
                              true, true);
        cds.setPricingEngine(ext::shared_ptr<PricingEngine>(
                         new MidPointCdsEngine(probabilityCurve,
                                               recoveryRate, discountCurve)));

        Real NPV = cds.NPV();
        Rate flatRate = cds.impliedHazardRate(NPV, discountCurve,
                                              dayCounter,
                                              recoveryRate);

        if (flatRate < h1 || flatRate > h2) {
            BOOST_ERROR("implied hazard rate outside expected range\n"
                        << "    maturity: " << n << " years\n"
                        << "    expected minimum: " << h1 << "\n"
                        << "    expected maximum: " << h2 << "\n"
                        << "    implied rate:     " << flatRate);
        }

        if (n > 6 && flatRate < latestRate) {
            BOOST_ERROR("implied hazard rate decreasing with swap maturity\n"
                        << "    maturity: " << n << " years\n"
                        << "    previous rate: " << latestRate << "\n"
                        << "    implied rate:  " << flatRate);
        }

        latestRate = flatRate;

        RelinkableHandle<DefaultProbabilityTermStructure> probability;
        probability.linkTo(ext::shared_ptr<DefaultProbabilityTermStructure>(
         new FlatHazardRate(
           today,
           Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(flatRate))),
           dayCounter)));

        CreditDefaultSwap cds2(Protection::Seller, notional, fixedRate,
                               schedule, convention, cdsDayCount,
                               true, true);
        cds2.setPricingEngine(ext::shared_ptr<PricingEngine>(
                               new MidPointCdsEngine(probability,recoveryRate,
                                                     discountCurve)));

        Real NPV2 = cds2.NPV();
        Real tolerance = 1.0;
        if (std::fabs(NPV-NPV2) > tolerance) {
            BOOST_ERROR("failed to reproduce NPV with implied rate\n"
                        << "    expected:   " << NPV << "\n"
                        << "    calculated: " << NPV2);
        }
    }
}

BOOST_AUTO_TEST_CASE(testFairSpread) {

    BOOST_TEST_MESSAGE(
        "Testing fair-spread calculation for credit-default swaps...");

    // Initialize curves
    Calendar calendar = TARGET();
    Date today = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today;

    Handle<Quote> hazardRate = Handle<Quote>(
                ext::shared_ptr<Quote>(new SimpleQuote(0.01234)));
    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(
        ext::shared_ptr<DefaultProbabilityTermStructure>(
                   new FlatHazardRate(0, calendar, hazardRate, Actual360())));

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.06,Actual360())));

    // Build the schedule
    Date issueDate = calendar.advance(today, -1, Years);
    Date maturity = calendar.advance(issueDate, 10, Years);
    BusinessDayConvention convention = Following;

    Schedule schedule =
        MakeSchedule().from(issueDate)
                      .to(maturity)
                      .withFrequency(Quarterly)
                      .withCalendar(calendar)
                      .withTerminationDateConvention(convention)
                      .withRule(DateGeneration::TwentiethIMM);

    // Build the CDS
    Rate fixedRate = 0.001;
    DayCounter dayCount = Actual360();
    Real notional = 10000.0;
    Real recoveryRate = 0.4;

    ext::shared_ptr<PricingEngine> engine(
          new MidPointCdsEngine(probabilityCurve,recoveryRate,discountCurve));

    CreditDefaultSwap cds(Protection::Seller, notional, fixedRate,
                          schedule, convention, dayCount, true, true);
    cds.setPricingEngine(engine);

    Rate fairRate = cds.fairSpread();

    CreditDefaultSwap fairCds(Protection::Seller, notional, fairRate,
                              schedule, convention, dayCount, true, true);
    fairCds.setPricingEngine(engine);

    Real fairNPV = fairCds.NPV();
    Real tolerance = 1e-9;

    if (std::fabs(fairNPV) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce null NPV with calculated fair spread\n"
            << "    calculated spread: " << io::rate(fairRate) << "\n"
            << "    calculated NPV:    " << fairNPV);
}

BOOST_AUTO_TEST_CASE(testFairUpfront) {

    BOOST_TEST_MESSAGE(
        "Testing fair-upfront calculation for credit-default swaps...");

    // Initialize curves
    Calendar calendar = TARGET();
    Date today = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = today;

    Handle<Quote> hazardRate = Handle<Quote>(
                ext::shared_ptr<Quote>(new SimpleQuote(0.01234)));
    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    probabilityCurve.linkTo(
        ext::shared_ptr<DefaultProbabilityTermStructure>(
                   new FlatHazardRate(0, calendar, hazardRate, Actual360())));

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(ext::shared_ptr<YieldTermStructure>(
                            new FlatForward(today,0.06,Actual360())));

    // Build the schedule
    Date issueDate = today;
    Date maturity = calendar.advance(issueDate, 10, Years);
    BusinessDayConvention convention = Following;

    Schedule schedule =
        MakeSchedule().from(issueDate)
                      .to(maturity)
                      .withFrequency(Quarterly)
                      .withCalendar(calendar)
                      .withTerminationDateConvention(convention)
                      .withRule(DateGeneration::TwentiethIMM);

    // Build the CDS
    Rate fixedRate = 0.05;
    Rate upfront = 0.001;
    DayCounter dayCount = Actual360();
    Real notional = 10000.0;
    Real recoveryRate = 0.4;

    ext::shared_ptr<PricingEngine> engine(
          new MidPointCdsEngine(probabilityCurve, recoveryRate,
                                discountCurve, true));

    CreditDefaultSwap cds(Protection::Seller, notional, upfront, fixedRate,
                          schedule, convention, dayCount, true, true);
    cds.setPricingEngine(engine);

    Rate fairUpfront = cds.fairUpfront();

    CreditDefaultSwap fairCds(Protection::Seller, notional,
                              fairUpfront, fixedRate,
                              schedule, convention, dayCount, true, true);
    fairCds.setPricingEngine(engine);

    Real fairNPV = fairCds.NPV();
    Real tolerance = 1e-9;

    if (std::fabs(fairNPV) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce null NPV with calculated fair upfront\n"
            << "    calculated upfront: " << io::rate(fairUpfront) << "\n"
            << "    calculated NPV:     " << fairNPV);

    // same with null upfront to begin with
    upfront = 0.0;
    CreditDefaultSwap cds2(Protection::Seller, notional, upfront, fixedRate,
                           schedule, convention, dayCount, true, true);
    cds2.setPricingEngine(engine);

    fairUpfront = cds2.fairUpfront();

    CreditDefaultSwap fairCds2(Protection::Seller, notional,
                               fairUpfront, fixedRate,
                               schedule, convention, dayCount, true, true);
    fairCds2.setPricingEngine(engine);

    fairNPV = fairCds2.NPV();

    if (std::fabs(fairNPV) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce null NPV with calculated fair upfront\n"
            << "    calculated upfront: " << io::rate(fairUpfront) << "\n"
            << "    calculated NPV:     " << fairNPV);
}

BOOST_AUTO_TEST_CASE(testIsdaEngine) {

    BOOST_TEST_MESSAGE(
        "Testing ISDA engine calculations for credit-default swaps...");

    bool usingAtParCoupons  = IborCoupon::Settings::instance().usingAtParCoupons();

    Date tradeDate(21, May, 2009);
    Settings::instance().evaluationDate() = tradeDate;


    //build an ISDA compliant yield curve
    //data comes from Markit published rates
    std::vector<ext::shared_ptr<RateHelper> > isdaRateHelpers;
    int dep_tenors[] = {1, 2, 3, 6, 9, 12};
    double dep_quotes[] = {0.003081,
                           0.005525,
                           0.007163,
                           0.012413,
                           0.014,
                           0.015488};

    for(size_t i = 0; i < sizeof(dep_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<DepositRateHelper>(
                                     dep_quotes[i], dep_tenors[i] * Months, 2,
                                     WeekendsOnly(), ModifiedFollowing,
                                     false, Actual360()
                                     )
            );
    }
    int swap_tenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 25, 30};
    double swap_quotes[] = {0.011907,
                            0.01699,
                            0.021198,
                            0.02444,
                            0.026937,
                            0.028967,
                            0.030504,
                            0.031719,
                            0.03279,
                            0.034535,
                            0.036217,
                            0.036981,
                            0.037246,
                            0.037605};

    ext::shared_ptr<IborIndex> isda_ibor = ext::make_shared<IborIndex>(
        "IsdaIbor", 3 * Months, 2, USDCurrency(), WeekendsOnly(),
        ModifiedFollowing, false, Actual360());
    for(size_t i = 0; i < sizeof(swap_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<SwapRateHelper>(
                                      swap_quotes[i], swap_tenors[i] * Years,
                                      WeekendsOnly(),
                                      Semiannual,
                                      ModifiedFollowing, Thirty360(Thirty360::BondBasis), isda_ibor
                                      )
            );
    }

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
                0, WeekendsOnly(), isdaRateHelpers, Actual365Fixed())
        );


    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    Date termDates[] = {Date(20, June, 2010),
                        Date(20, June, 2011),
                        Date(20, June, 2012),
                        Date(20, June, 2016),
                        Date(20, June, 2019)};
    Rate spreads[] = {0.001, 0.1};
    Rate recoveries[] = {0.2, 0.4};

    double markitValues[] = {
         -97798.29358, //0.001
         -97776.11889, //0.001
         914971.5977,  //0.1
         894985.6298,  //0.1
        -186921.3594,  //0.001
        -186839.8148,  //0.001
        1646623.672,   //0.1
        1579803.626,   //0.1
        -274298.9203,
        -274122.4725,
        2279730.93,
        2147972.527,
        -592420.2297,
        -591571.2294,
        3993550.206,
        3545843.418,
        -797501.1422,
        -795915.9787,
        4702034.688,
        4042340.999
    };

    /* When using indexes coupons, the risk-free curve is a bit off.
       We might skip the tests altogether and rely on running them
       with indexed coupons disabled, but leaving them can be useful anyway. */
    Real tolerance = usingAtParCoupons ? 1.0e-6 : 1.0e-3;

    size_t l = 0;

    for (auto termDate : termDates) {
        for (Real spread : spreads) {
            for (Real& recovery : recoveries) {

                ext::shared_ptr<CreditDefaultSwap> quotedTrade =
                    MakeCreditDefaultSwap(termDate, spread).withNominal(10000000.);

                Rate h = quotedTrade->impliedHazardRate(0., discountCurve, Actual365Fixed(),
                                                        recovery, 1e-10, CreditDefaultSwap::ISDA);

                probabilityCurve.linkTo(
                    ext::make_shared<FlatHazardRate>(0, WeekendsOnly(), h, Actual365Fixed()));

                ext::shared_ptr<IsdaCdsEngine> engine = ext::make_shared<IsdaCdsEngine>(
                    probabilityCurve, recovery, discountCurve, ext::nullopt, IsdaCdsEngine::Taylor,
                    IsdaCdsEngine::HalfDayBias, IsdaCdsEngine::Piecewise);

                ext::shared_ptr<CreditDefaultSwap> conventionalTrade =
                    MakeCreditDefaultSwap(termDate, 0.01)
                        .withNominal(10000000.)
                        .withPricingEngine(engine);

                QL_CHECK_CLOSE(conventionalTrade->notional() * conventionalTrade->fairUpfront(),
                               markitValues[l], tolerance);

                // Now testing that with the calculated fair-upfront, both Buyer and Seller sides
                // price close to zero
                ext::shared_ptr<CreditDefaultSwap> conventionalTradeBuy =
                    MakeCreditDefaultSwap(termDate, 0.01)
                        .withNominal(10000000.)
                        .withUpfrontRate(conventionalTrade->fairUpfront())
                        .withSide(Protection::Buyer)
                        .withPricingEngine(engine);

                QL_CHECK_SMALL(conventionalTradeBuy->NPV(), tolerance);

                ext::shared_ptr<CreditDefaultSwap> conventionalTradeSell =
                    MakeCreditDefaultSwap(termDate, 0.01)
                        .withNominal(10000000.)
                        .withUpfrontRate(conventionalTrade->fairUpfront())
                        .withSide(Protection::Seller)
                        .withPricingEngine(engine);

                QL_CHECK_SMALL(conventionalTradeSell->NPV(), tolerance);

                l++;
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testAccrualRebateAmounts) {

    BOOST_TEST_MESSAGE("Testing accrual rebate amounts on credit default swaps...");

    // The accrual values are taken from various test results on the ISDA CDS model website
    // https://www.cdsmodel.com/cdsmodel/documentation.html.

    // Inputs
    Real notional = 10000000;
    Real spread = 0.0100;
    Date maturity(20, Jun, 2014);

    // key is trade date and value is expected accrual
    typedef map<Date, Real> InputData;
    InputData inputs = {
        {Date(18, Mar, 2009), 24166.67},
        {Date(19, Mar, 2009), 0.00},
        {Date(20, Mar, 2009), 277.78},
        {Date(23, Mar, 2009), 1111.11},
        {Date(19, Jun, 2009), 25555.56},
        {Date(20, Jun, 2009), 25833.33},
        {Date(21, Jun, 2009), 0.00},
        {Date(22, Jun, 2009), 277.78},
        {Date(18, Jun, 2014), 25277.78},
        {Date(19, Jun, 2014), 25555.56}
    };

    for (auto& input: inputs) {
        Settings::instance().evaluationDate() = input.first;
        CreditDefaultSwap cds = MakeCreditDefaultSwap(maturity, spread)
            .withNominal(notional);
        QL_CHECK_SMALL(input.second - cds.accrualRebate()->amount(), 0.01);
    }
}

BOOST_AUTO_TEST_CASE(testIsdaCalculatorReconcileSingleQuote) {
    BOOST_TEST_MESSAGE(
        "Testing ISDA engine calculations for a single credit-default swap record (reconciliation)...");

    Date tradeDate(26, July, 2021);
    Settings::instance().evaluationDate() = tradeDate;

    //build an ISDA compliant yield curve
    //data comes from Markit published rates
    std::vector<ext::shared_ptr<RateHelper> > isdaRateHelpers;
    int dep_tenors[] = {1, 3, 6, 12};
    double dep_quotes[] = {-0.0056,-0.005440,-0.005190,-0.004930};

    for(size_t i = 0; i < sizeof(dep_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<DepositRateHelper>(
                                     dep_quotes[i], dep_tenors[i] * Months, 2,
                                     WeekendsOnly(), ModifiedFollowing,
                                     false, Actual360()
                                     )
            );
    }
    int swap_tenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30};
    double swap_quotes[] = {-0.004820,
                            -0.004420,
                            -0.003990,
                            -0.003520,
                            -0.002970,
                            -0.002370,
                            -0.001760,
                            -0.001140,
                            -0.000540,
                            0.000570,
                            0.001880,
                            0.002940,
                            0.002820};

    ext::shared_ptr<IborIndex> isda_ibor = ext::make_shared<IborIndex>(
        "IsdaIbor", 6 * Months, 2, EURCurrency(), WeekendsOnly(),
        ModifiedFollowing, false, Actual360());
    for(size_t i = 0; i < sizeof(swap_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<SwapRateHelper>(
                                      swap_quotes[i], swap_tenors[i] * Years,
                                      WeekendsOnly(),
                                      Annual,
                                      ModifiedFollowing, Thirty360(Thirty360::BondBasis), isda_ibor
                                      )
            );
    }

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
                0, WeekendsOnly(), isdaRateHelpers, Actual365Fixed())
        );

    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    Date instrumentMaturity = Date(20, June, 2026);
    Rate coupon = 0.01, conventionalSpread = 0.006713, recovery = 0.4;
    double nominal = 1e6, markitValue = -16070.7, expected_accrual = 1000, tolerance = 1.0e-3;

    ext::shared_ptr<CreditDefaultSwap> quotedTrade =
        MakeCreditDefaultSwap(instrumentMaturity, conventionalSpread).withNominal(nominal);

    Rate h = quotedTrade->impliedHazardRate(0., discountCurve, Actual365Fixed(),
                                            recovery, 1e-10, CreditDefaultSwap::ISDA);

    probabilityCurve.linkTo(
        ext::make_shared<FlatHazardRate>(0, WeekendsOnly(), h, Actual365Fixed()));

    ext::shared_ptr<IsdaCdsEngine> engine = ext::make_shared<IsdaCdsEngine>(
        probabilityCurve, recovery, discountCurve, ext::nullopt, IsdaCdsEngine::Taylor,
        IsdaCdsEngine::HalfDayBias, IsdaCdsEngine::Piecewise);

    ext::shared_ptr<CreditDefaultSwap> conventionalTrade =
        MakeCreditDefaultSwap(instrumentMaturity, coupon)
            .withNominal(nominal)
            .withPricingEngine(engine);


    Real npv = conventionalTrade->NPV();
    Real calculated_upfront = conventionalTrade->notional() * conventionalTrade->fairUpfront();
    Real df = calculated_upfront / npv; // to take into account of the discount to cash settlement
    Real derived_accrual =
        df * (npv -
                                   conventionalTrade->defaultLegNPV() -
                                   conventionalTrade->couponLegNPV());

    Real calculated_accrual = conventionalTrade->accrualRebate()->amount();

    auto settlement_date = conventionalTrade->accrualRebate()->date();

    QL_CHECK_CLOSE(npv, markitValue, tolerance);

    QL_CHECK_CLOSE(calculated_upfront, df * markitValue, tolerance);

    QL_CHECK_CLOSE(derived_accrual, expected_accrual, tolerance);

    QL_CHECK_CLOSE(calculated_accrual, expected_accrual, tolerance);

    BOOST_CHECK_EQUAL(settlement_date, WeekendsOnly().advance(tradeDate,3, TimeUnit::Days));

}

BOOST_AUTO_TEST_CASE(testIsdaCalculatorReconcileSingleWithIssueDateInThePast) {
    BOOST_TEST_MESSAGE(
        "Testing ISDA engine calculations for a single credit-default swap with issue date in the past...");

    Date valueDate(26, July, 2021);
    Settings::instance().evaluationDate() = valueDate;

    //this is not IMM date but the settlement date is in the past so the accrual rebate
    //should not be part of the NPV
    Date tradeDate(20, July, 2019);

    //build an ISDA compliant yield curve
    //data comes from Markit published rates
    std::vector<ext::shared_ptr<RateHelper> > isdaRateHelpers;
    int dep_tenors[] = {1, 3, 6, 12};
    double dep_quotes[] = {-0.0056,-0.005440,-0.005190,-0.004930};

    for(size_t i = 0; i < sizeof(dep_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<DepositRateHelper>(
                                     dep_quotes[i], dep_tenors[i] * Months, 2,
                                     WeekendsOnly(), ModifiedFollowing,
                                     false, Actual360()
                                     )
            );
    }
    int swap_tenors[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30};
    double swap_quotes[] = {-0.004820,
                            -0.004420,
                            -0.003990,
                            -0.003520,
                            -0.002970,
                            -0.002370,
                            -0.001760,
                            -0.001140,
                            -0.000540,
                            0.000570,
                            0.001880,
                            0.002940,
                            0.002820};

    ext::shared_ptr<IborIndex> isda_ibor = ext::make_shared<IborIndex>(
        "IsdaIbor", 6 * Months, 2, EURCurrency(), WeekendsOnly(),
        ModifiedFollowing, false, Actual360());
    for(size_t i = 0; i < sizeof(swap_tenors) / sizeof(int); i++) {
        isdaRateHelpers.push_back(ext::make_shared<SwapRateHelper>(
                                      swap_quotes[i], swap_tenors[i] * Years,
                                      WeekendsOnly(),
                                      Annual,
                                      ModifiedFollowing, Thirty360(Thirty360::BondBasis), isda_ibor
                                      )
            );
    }

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
            ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(
                0, WeekendsOnly(), isdaRateHelpers, Actual365Fixed())
        );

    RelinkableHandle<DefaultProbabilityTermStructure> probabilityCurve;
    Date instrumentMaturity = Date(20, June, 2026);
    Rate coupon = 0.01, conventionalSpread = 0.006713, recovery = 0.4;

    //because there is no accrual involved, the markit value is decreased as compared to the
    //previous test (old_markit_value - old_accrual or -16070.7 - 1000)
    double nominal = 1e6, markitValue = -17070.77, expected_accrual = 0, tolerance = 1.0e-3;

    ext::shared_ptr<CreditDefaultSwap> quotedTrade =
        MakeCreditDefaultSwap(instrumentMaturity, conventionalSpread)
        .withNominal(nominal);

    Rate h = quotedTrade->impliedHazardRate(0., discountCurve, Actual365Fixed(),
                                            recovery, 1e-10, CreditDefaultSwap::ISDA);

    probabilityCurve.linkTo(
        ext::make_shared<FlatHazardRate>(0, WeekendsOnly(), h, Actual365Fixed()));

    ext::shared_ptr<IsdaCdsEngine> engine = ext::make_shared<IsdaCdsEngine>(
        probabilityCurve, recovery, discountCurve, ext::nullopt, IsdaCdsEngine::Taylor,
        IsdaCdsEngine::HalfDayBias, IsdaCdsEngine::Piecewise);

    ext::shared_ptr<CreditDefaultSwap> conventionalTrade =
        MakeCreditDefaultSwap(instrumentMaturity, coupon)
            .withNominal(nominal)
            .withPricingEngine(engine)
            .withTradeDate(tradeDate);


    Real npv = conventionalTrade->NPV();
    Real calculated_accrual = npv -
                                conventionalTrade->defaultLegNPV() -
                                conventionalTrade->couponLegNPV();

    QL_CHECK_CLOSE(npv, markitValue, tolerance);

    QL_CHECK_CLOSE(calculated_accrual, expected_accrual, tolerance);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
