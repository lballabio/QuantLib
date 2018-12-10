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

#include "creditdefaultswap.hpp"
#include "utilities.hpp"
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
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/thirty360.hpp>

#include <iomanip>
#include <iostream>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void CreditDefaultSwapTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing credit-default swap against cached values...");

    SavedSettings backup;

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


void CreditDefaultSwapTest::testCachedMarketValue() {

    BOOST_TEST_MESSAGE(
        "Testing credit-default swap against cached market values...");

    SavedSettings backup;

    Settings::instance().evaluationDate() = Date(9,June,2006);
    Date evalDate = Settings::instance().evaluationDate();
    Calendar calendar = UnitedStates();

    std::vector<Date> discountDates;
    discountDates.push_back(evalDate);
    discountDates.push_back(calendar.advance(evalDate, 1, Weeks,  ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 1, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 2, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 3, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 6, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,12, Months, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 2, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 3, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 4, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 5, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 6, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 7, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 8, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate, 9, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,10, Years, ModifiedFollowing));
    discountDates.push_back(calendar.advance(evalDate,15, Years, ModifiedFollowing));

    std::vector<DiscountFactor> dfs;
    dfs.push_back(1.0);
    dfs.push_back(0.9990151375768731);
    dfs.push_back(0.99570502636871183);
    dfs.push_back(0.99118260474528685);
    dfs.push_back(0.98661167950906203);
    dfs.push_back(0.9732592953359388 );
    dfs.push_back(0.94724424481038083);
    dfs.push_back(0.89844996737120875  );
    dfs.push_back(0.85216647839921411  );
    dfs.push_back(0.80775477692556874  );
    dfs.push_back(0.76517289234200347  );
    dfs.push_back(0.72401019553182933  );
    dfs.push_back(0.68503909569219212  );
    dfs.push_back(0.64797499814013748  );
    dfs.push_back(0.61263171936255534  );
    dfs.push_back(0.5791942350748791   );
    dfs.push_back(0.43518868769953606  );

    const DayCounter& curveDayCounter=Actual360();

    RelinkableHandle<YieldTermStructure> discountCurve;
    discountCurve.linkTo(
        ext::shared_ptr<YieldTermStructure>(
            new DiscountCurve(discountDates, dfs, curveDayCounter)));

    DayCounter dayCounter = Thirty360();
    std::vector<Date> dates;
    dates.push_back(evalDate);
    dates.push_back(calendar.advance(evalDate, 6, Months, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 1, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 2, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 3, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 4, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 5, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate, 7, Years, ModifiedFollowing));
    dates.push_back(calendar.advance(evalDate,10, Years, ModifiedFollowing));

    std::vector<Probability> defaultProbabilities;
    defaultProbabilities.push_back(0.0000);
    defaultProbabilities.push_back(0.0047);
    defaultProbabilities.push_back(0.0093);
    defaultProbabilities.push_back(0.0286);
    defaultProbabilities.push_back(0.0619);
    defaultProbabilities.push_back(0.0953);
    defaultProbabilities.push_back(0.1508);
    defaultProbabilities.push_back(0.2288);
    defaultProbabilities.push_back(0.3666);

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
                                                             Thirty360())));

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


void CreditDefaultSwapTest::testImpliedHazardRate() {

    BOOST_TEST_MESSAGE("Testing implied hazard-rate for credit-default swaps...");

    SavedSettings backup;

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


void CreditDefaultSwapTest::testFairSpread() {

    BOOST_TEST_MESSAGE(
        "Testing fair-spread calculation for credit-default swaps...");

    SavedSettings backup;

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
    Real tolerance = 1e-10;

    if (std::fabs(fairNPV) > tolerance)
        BOOST_ERROR(
            "Failed to reproduce null NPV with calculated fair spread\n"
            << "    calculated spread: " << io::rate(fairRate) << "\n"
            << "    calculated NPV:    " << fairNPV);
}

void CreditDefaultSwapTest::testFairUpfront() {

    BOOST_TEST_MESSAGE(
        "Testing fair-upfront calculation for credit-default swaps...");

    SavedSettings backup;

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
    Real tolerance = 1e-10;

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

void CreditDefaultSwapTest::testIsdaEngine() {

    BOOST_TEST_MESSAGE(
        "Testing ISDA engine calculations for credit-default swaps...");

    SavedSettings backup;

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
                                      ModifiedFollowing, Thirty360(), isda_ibor
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

    double markitValues[] = {97798.29358, //0.001
                             97776.11889, //0.001
                             -914971.5977, //0.1
                             -894985.6298, //0.1
                             186921.3594, //0.001
                             186839.8148, //0.001
                             -1646623.672, //0.1
                             -1579803.626, //0.1
                             274298.9203,
                             274122.4725,
                             -2279730.93,
                             -2147972.527,
                             592420.2297,
                             591571.2294,
                             -3993550.206,
                             -3545843.418,
                             797501.1422,
                             795915.9787,
                             -4702034.688,
                             -4042340.999};
    #ifndef QL_USE_INDEXED_COUPON
    Real tolerance = 1.0e-6;
    #else
    /* The risk-free curve is a bit off. We might skip the tests
       altogether and rely on running them with indexed coupons
       disabled, but leaving them can be useful anyway. */
    Real tolerance = 1.0e-3;
    #endif

    size_t l = 0;

    for(size_t i = 0; i < sizeof(termDates) / sizeof(Date); i++) {
        for(size_t j = 0; j < 2; j++) {
            for(size_t k = 0; k < 2; k++) {

            ext::shared_ptr<CreditDefaultSwap> quotedTrade =
                MakeCreditDefaultSwap(termDates[i], spreads[j])
                .withNominal(10000000.);

            Rate h = quotedTrade->impliedHazardRate(0.,
                                                    discountCurve,
                                                    Actual365Fixed(),
                                                    recoveries[k],
                                                    1e-10,
                                                    CreditDefaultSwap::ISDA);

            probabilityCurve.linkTo(
                ext::make_shared<FlatHazardRate>(
                    0, WeekendsOnly(), h, Actual365Fixed())
                );

            ext::shared_ptr<IsdaCdsEngine> engine = ext::make_shared<IsdaCdsEngine>(
                probabilityCurve, recoveries[k], discountCurve,
                boost::none, IsdaCdsEngine::Taylor, IsdaCdsEngine::HalfDayBias,
                IsdaCdsEngine::Piecewise);

            ext::shared_ptr<CreditDefaultSwap> conventionalTrade =
                MakeCreditDefaultSwap(termDates[i], 0.01)
                .withNominal(10000000.)
                .withPricingEngine(engine);

            BOOST_CHECK_CLOSE(conventionalTrade->notional() * conventionalTrade->fairUpfront(),
                              markitValues[l],
                              tolerance);

            l++;

            }
        }
    }

}

test_suite* CreditDefaultSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Credit-default swap tests");
    suite->add(QUANTLIB_TEST_CASE(&CreditDefaultSwapTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(
                              &CreditDefaultSwapTest::testCachedMarketValue));
    suite->add(QUANTLIB_TEST_CASE(
                              &CreditDefaultSwapTest::testImpliedHazardRate));
    suite->add(QUANTLIB_TEST_CASE(&CreditDefaultSwapTest::testFairSpread));
    suite->add(QUANTLIB_TEST_CASE(&CreditDefaultSwapTest::testFairUpfront));
    suite->add(QUANTLIB_TEST_CASE(&CreditDefaultSwapTest::testIsdaEngine));
    return suite;
}
