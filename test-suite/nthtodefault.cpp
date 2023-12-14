/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/currencies/europe.hpp>
#include <ql/experimental/credit/constantlosslatentmodel.hpp>
#include <ql/experimental/credit/integralntdengine.hpp>
#include <ql/experimental/credit/nthtodefault.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/integralcdsengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <iostream>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(NthToDefaultTests)

#ifndef QL_PATCH_SOLARIS

namespace {

    struct hwDatum {
        Size rank;
        Real spread[3];
    };

    /* Spread (bp p.a.) to buy protection for the nth to default from
       a basket of 10 names. All pairs have same correlation, 0 in
       column 2, 0.3 in column 3, 0.6 in column 4. Default intensity
       for all names is constant at 0.01, maturity 5 years, equal
       notional amounts.
    */
    hwDatum hwData[] = {
        { 1, { 603, 440, 293 } },
        { 2, {  98, 139, 137 } },
        { 3, {  12,  53,  79 } },
        { 4, {   1,  21,  49 } },
        { 5, {   0,   8,  31 } },
        { 6, {   0,   3,  19 } },
        { 7, {   0,   1,  12 } },
        { 8, {   0,   0,   7 } },
        { 9, {   0,   0,   3 } },
        {10, {   0,   0,   1 } }
    };


    Real hwCorrelation[] = { 0.0, 0.3, 0.6 };


    struct hwDatumDist {
        Size rank;
        Real spread[4];
    };

    // HW Table 3, Nth to Default Basket
    // corr = 0.3
    // NM/NZ
    // rank inf/inf 5/inf inf/5 5/5
    hwDatumDist hwDataDist[] = {
        { 1, { 440, 419, 474, 455 } },
        { 2, { 139, 127, 127, 116 } },
        { 3, {  53,  51,  44,  44 } },
        { 4, {  21,  24,  18,  22 } },
        { 5, {   8,  13,   7,  13 } },
        { 6, {   3,   8,   3,   8 } },
        { 7, {   1,   5,   1,   5 } },
        { 8, {   0,   3,   0,   4 } },
        { 9, {   0,   2,   0,   2 } },
        {10, {   0,   1,   0,   1 } }
    };

}

#endif

#ifndef QL_PATCH_SOLARIS

BOOST_AUTO_TEST_CASE(testGauss, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Gaussian copula...");

    /*************************
     * Tolerances
     */
    Real relTolerance = 0.015; // relative difference
    Real absTolerance = 1; // absolute difference in bp

    Period timeUnit = 1*Weeks; // required to reach accuracy

    Size names = 10;
    QL_REQUIRE (LENGTH(hwData) == names, "hwData length does not match");

    Real rate = 0.05;
    DayCounter dc = Actual365Fixed();
    Compounding cmp = Continuous; // Simple;

    Real recovery = 0.4;
    std::vector<Real> lambda (names, 0.01);

    Real namesNotional = 100.0;

    Schedule schedule = MakeSchedule().from(Date (1, September, 2006))
                                      .to(Date (1, September, 2011))
                                      .withTenor(3*Months)
                                      .withCalendar(TARGET());

    Date asofDate(31, August, 2006);

    Settings::instance().evaluationDate() = asofDate;

    std::vector<Date> gridDates = {
        asofDate,
        TARGET().advance (asofDate, Period (1, Years)),
        TARGET().advance (asofDate, Period (5, Years)),
        TARGET().advance (asofDate, Period (7, Years))
    };

    ext::shared_ptr<YieldTermStructure> yieldPtr (
                                   new FlatForward (asofDate, rate, dc, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    std::vector<Handle<DefaultProbabilityTermStructure> > probabilities;
    Period maxTerm (10, Years);
    for (Real i : lambda) {
        Handle<Quote> h(ext::shared_ptr<Quote>(new SimpleQuote(i)));
        ext::shared_ptr<DefaultProbabilityTermStructure> ptr (
                                         new FlatHazardRate(asofDate, h, dc));
        probabilities.emplace_back(ptr);
    }

    ext::shared_ptr<SimpleQuote> simpleQuote (new SimpleQuote(0.0));
    Handle<Quote> correlationHandle (simpleQuote);

    ext::shared_ptr<DefaultLossModel> copula( new 
        ConstantLossModel<GaussianCopulaPolicy>( correlationHandle, 
        std::vector<Real>(names, recovery), 
        LatentModelIntegrationType::GaussianQuadrature, names, 
        GaussianCopulaPolicy::initTraits()));

    /* If you like the action you can price with the simulation engine below
    instead below. But you need at least 1e6 simulations to pass the pricing 
    error tests
    */
    //ext::shared_ptr<GaussianDefProbLM> gLM(
    //    ext::make_shared<GaussianDefProbLM>(correlationHandle, names,
    //    LatentModelIntegrationType::GaussianQuadrature,
    //    // g++ requires this when using make_shared
    //    GaussianCopulaPolicy::initTraits()));
    //Size numSimulations = 1000000;
    //// Size numCoresUsed = 4; use your are in the multithread branch
    //// Sobol, many cores
    //ext::shared_ptr<RandomDefaultLM<GaussianCopulaPolicy> > copula( 
    //    new RandomDefaultLM<GaussianCopulaPolicy>(gLM, 
    //        std::vector<Real>(names, recovery), numSimulations, 1.e-6, 
    //        2863311530));

    // Set up pool and basket
    std::vector<std::string> namesIds;
    for(Size i=0; i<names; i++)
        namesIds.push_back(std::string("Name") + std::to_string(i));

    std::vector<Issuer> issuers;
    for(Size i=0; i<names; i++) {
        std::vector<QuantLib::Issuer::key_curve_pair> curves(1, 
            std::make_pair(NorthAmericaCorpDefaultKey(
                EURCurrency(), QuantLib::SeniorSec,
                Period(), 1. // amount threshold
                ), probabilities[i]));
        issuers.emplace_back(curves);
    }

    ext::shared_ptr<Pool> thePool = ext::make_shared<Pool>();
    for(Size i=0; i<names; i++)
        thePool->add(namesIds[i], issuers[i], NorthAmericaCorpDefaultKey(
                EURCurrency(), QuantLib::SeniorSec, Period(), 1.));

    std::vector<DefaultProbKey> defaultKeys(probabilities.size(), 
        NorthAmericaCorpDefaultKey(EURCurrency(), SeniorSec, Period(), 1.));

    ext::shared_ptr<Basket> basket(new Basket(asofDate, namesIds, 
        std::vector<Real>(names, namesNotional/names), thePool, 0., 1.));

    ext::shared_ptr<PricingEngine> engine(
        new IntegralNtdEngine(timeUnit, yieldHandle));

    std::vector<NthToDefault> ntd;
    for (Size i = 1; i <= probabilities.size(); i++) {
        ntd.emplace_back(basket, i, Protection::Seller, schedule, 0.0, 0.02, Actual360(),
                         namesNotional * names, true);
        ntd.back().setPricingEngine(engine);
    }

    QL_REQUIRE (LENGTH(hwCorrelation) == 3,
                "correlation length does not match");

    Real diff, maxDiff = 0;

    basket->setLossModel(copula);
    
    for (Size j = 0; j < LENGTH(hwCorrelation); j++) {
        simpleQuote->setValue (hwCorrelation[j]);
        for (Size i = 0; i < ntd.size(); i++) {
            QL_REQUIRE (ntd[i].rank() == hwData[i].rank, "rank does not match");
            QL_REQUIRE (LENGTH(hwCorrelation) == LENGTH(hwData[i].spread),
                        "vector length does not match");
            diff = 1e4 * ntd[i].fairPremium() - hwData[i].spread[j];
            maxDiff = std::max(maxDiff, fabs (diff));
            BOOST_CHECK_MESSAGE (fabs(diff/hwData[i].spread[j]) < relTolerance
                                 || fabs(diff) < absTolerance,
                                 "tolerance " << relTolerance << "|"
                                 << absTolerance << " exceeded");
        }
    }
}
BOOST_AUTO_TEST_CASE(testStudent, *precondition(if_speed(Slow))) {

    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Student copula...");

    /*************************
     * Tolerances
     */
    Real relTolerance = 0.017; // relative difference
    Real absTolerance = 1; // absolute difference in bp

    Period timeUnit = 1*Weeks; // required to reach accuracy

    Size names = 10;
    QL_REQUIRE (LENGTH(hwDataDist) == names, "hwDataDist length does not match");

    Real rate = 0.05;
    DayCounter dc = Actual365Fixed();
    Compounding cmp = Continuous; // Simple;


    Real recovery = 0.4;
    std::vector<Real> lambda (names, 0.01);

    Real namesNotional = 100.0;

    Schedule schedule = MakeSchedule().from(Date (1, September, 2006))
                                      .to(Date (1, September, 2011))
                                      .withTenor(3*Months)
                                      .withCalendar(TARGET());

    Date asofDate(31, August, 2006);

    Settings::instance().evaluationDate() = asofDate;

    std::vector<Date> gridDates {
        asofDate,
        TARGET().advance (asofDate, Period (1, Years)),
        TARGET().advance (asofDate, Period (5, Years)),
        TARGET().advance (asofDate, Period (7, Years))
    };

    ext::shared_ptr<YieldTermStructure> yieldPtr (
                                new FlatForward (asofDate, rate, dc, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    std::vector<Handle<DefaultProbabilityTermStructure> > probabilities;
    Period maxTerm (10, Years);
    for (Real i : lambda) {
        Handle<Quote> h(ext::shared_ptr<Quote>(new SimpleQuote(i)));
        ext::shared_ptr<DefaultProbabilityTermStructure> ptr (
                                         new FlatHazardRate(asofDate, h, dc));
        probabilities.emplace_back(ptr);
    }

    ext::shared_ptr<SimpleQuote> simpleQuote (new SimpleQuote(0.0));
    Handle<Quote> correlationHandle (simpleQuote);

    TCopulaPolicy::initTraits iniT;
    iniT.tOrders = std::vector<QuantLib::Integer>(2,5);
    ext::shared_ptr<DefaultLossModel> copula( new 
        ConstantLossModel<TCopulaPolicy>( correlationHandle, 
        std::vector<Real>(names, recovery), 
        LatentModelIntegrationType::GaussianQuadrature, names, iniT));

    // Set up pool and basket
    std::vector<std::string> namesIds;
    for(Size i=0; i<names; i++)
        namesIds.push_back(std::string("Name") + std::to_string(i));

    std::vector<Issuer> issuers;
    for(Size i=0; i<names; i++) {
        std::vector<QuantLib::Issuer::key_curve_pair> curves(1, 
            std::make_pair(NorthAmericaCorpDefaultKey(
                EURCurrency(), QuantLib::SeniorSec,
                Period(), 1. // amount threshold
                ), probabilities[i]));
        issuers.emplace_back(curves);
    }

    ext::shared_ptr<Pool> thePool = ext::make_shared<Pool>();
    for(Size i=0; i<names; i++)
        thePool->add(namesIds[i], issuers[i], NorthAmericaCorpDefaultKey(
                EURCurrency(), QuantLib::SeniorSec, Period(), 1.));

    std::vector<DefaultProbKey> defaultKeys(probabilities.size(), 
        NorthAmericaCorpDefaultKey(EURCurrency(), SeniorSec, Period(), 1.));

    ext::shared_ptr<Basket> basket(new Basket(asofDate, namesIds, 
        std::vector<Real>(names, namesNotional/names), thePool, 0., 1.));

    ext::shared_ptr<PricingEngine> engine(
        new IntegralNtdEngine(timeUnit, yieldHandle));

    std::vector<NthToDefault> ntd;
    for (Size i = 1; i <= probabilities.size(); i++) {
        ntd.emplace_back(basket, i, Protection::Seller, schedule, 0.0, 0.02, Actual360(),
                         namesNotional * names, true);
        ntd.back().setPricingEngine(engine);
    }

    QL_REQUIRE (LENGTH(hwCorrelation) == 3,
                "correlation length does not match");

    Real maxDiff = 0;

    basket->setLossModel(copula);

    // This is the necessary code, but a proper hwData for the t copula is needed.
    // Real diff;
    // for (Size j = 0; j < LENGTH(hwCorrelation); j++) {
    //     simpleQuote->setValue (hwCorrelation[j]);
    //     for (Size i = 0; i < ntd.size(); i++) {
    //         QL_REQUIRE (ntd[i].rank() == hwData[i].rank, "rank does not match");
    //         QL_REQUIRE (LENGTH(hwCorrelation) == LENGTH(hwData[i].spread),
    //                     "vector length does not match");
    //         diff = 1e4 * ntd[i].fairPremium() - hwData[i].spread[j];
    //         maxDiff = std::max(maxDiff, fabs (diff));
    //         BOOST_CHECK_MESSAGE (fabs(diff/hwData[i].spread[j]) < relTolerance
    //                              || fabs(diff) < absTolerance,
    //                              "tolerance2 " << relTolerance << "|"
    //                              << absTolerance << " exceeded";
    //     }
    // }

    //instead of this BEGIN
    simpleQuote->setValue (0.3);
    
    for (Size i = 0; i < ntd.size(); i++) {
        QL_REQUIRE (ntd[i].rank() == hwDataDist[i].rank, "rank does not match");

        Real diff = 1e4 * ntd[i].fairPremium() - hwDataDist[i].spread[3];
        maxDiff = std::max(maxDiff, fabs (diff));
        BOOST_CHECK_MESSAGE (fabs(diff / hwDataDist[i].spread[3]) < relTolerance ||
                             fabs(diff) < absTolerance,
                             "tolerance " << relTolerance << "|"
                             << absTolerance << " exceeded" << i << "|"
                             << abs(diff) << "|" << hwDataDist[i].spread[3]);
    }
    //END
}

#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
