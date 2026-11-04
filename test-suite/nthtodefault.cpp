/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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
#include <iomanip>
#include <iostream>
#include <string>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(NthToDefaultTests)

#ifndef QL_PATCH_SOLARIS

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


BOOST_AUTO_TEST_CASE(testGauss) {
    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Gaussian copula...");

    /*************************
     * Tolerances
     */
    Real relTolerance = 0.015; // relative difference
    Real absTolerance = 1; // absolute difference in bp

    Period timeUnit = 1*Weeks; // required to reach accuracy

    Size names = 10;
    QL_REQUIRE (std::size(hwData) == names, "hwData length does not match");

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
    namesIds.reserve(names);
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

    static_assert(std::size(hwCorrelation) == 3,
                "correlation length does not match");

    Real diff, maxDiff = 0;

    basket->setLossModel(copula);

    for (Size j = 0; j < std::size(hwCorrelation); j++) {
        simpleQuote->setValue (hwCorrelation[j]);
        for (Size i = 0; i < ntd.size(); i++) {
            QL_REQUIRE(ntd[i].rank() == hwData[i].rank, "rank does not match");
            diff = 1e4 * ntd[i].fairPremium() - hwData[i].spread[j];
            maxDiff = std::max(maxDiff, fabs (diff));
            BOOST_CHECK_MESSAGE (fabs(diff/hwData[i].spread[j]) < relTolerance
                                 || fabs(diff) < absTolerance,
                                 "tolerance " << relTolerance << "|"
                                 << absTolerance << " exceeded");
        }
    }
}
BOOST_AUTO_TEST_CASE(testStudent) {

    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Student copula...");

    /*************************
     * Tolerances
     */
    Real relTolerance = 0.017; // relative difference
    Real absTolerance = 1; // absolute difference in bp

    Period timeUnit = 1*Weeks; // required to reach accuracy

    Size names = 10;
    QL_REQUIRE (std::size(hwDataDist) == names, "hwDataDist length does not match");

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
    namesIds.reserve(names);
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

    static_assert(std::size(hwCorrelation) == 3,
                "correlation length does not match");

    Real maxDiff = 0;

    basket->setLossModel(copula);

    // This is the necessary code, but a proper hwData for the t copula is needed.
    // Real diff;
    // for (Size j = 0; j < std::size(hwCorrelation); j++) {
    //     simpleQuote->setValue (hwCorrelation[j]);
    //     for (Size i = 0; i < ntd.size(); i++) {
    //         QL_REQUIRE (ntd[i].rank() == hwData[i].rank, "rank does not match");
    //         static_assert(std::size(hwCorrelation) == std::size(hwData[i].spread),
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

BOOST_AUTO_TEST_CASE(testProbAtLeastNEvents) {
    BOOST_TEST_MESSAGE("Testing probability of at least n defaults "
                       "against a direct enumeration...");

    // probability of at least n defaults among independent names with
    // default probabilities p, by enumeration of all configurations
    auto atLeastNEvents = [](const std::vector<Real>& p, Size n) {
        Real sum = 0.0;
        for (Size config = 0; config < (Size(1) << p.size()); config++) {
            Size defaults = 0;
            Real prob = 1.0;
            for (Size i = 0; i < p.size(); i++) {
                if (((config >> i) & 1) != 0U) {
                    defaults++;
                    prob *= p[i];
                } else {
                    prob *= 1.0 - p[i];
                }
            }
            if (defaults >= n)
                sum += prob;
        }
        return sum;
    };

    Date asofDate(31, August, 2006);
    Settings::instance().evaluationDate() = asofDate;
    DayCounter dc = Actual365Fixed();
    Date testDate = asofDate + 5*Years;

    std::vector<Real> lambda = {0.01, 0.07, 0.02, 0.15, 0.04, 0.10, 0.005};
    Size names = lambda.size();

    std::vector<std::string> namesIds;
    std::vector<Real> pUncond;
    ext::shared_ptr<Pool> thePool = ext::make_shared<Pool>();
    for (Size i = 0; i < names; i++) {
        namesIds.push_back(std::string("Name") + std::to_string(i));
        Handle<DefaultProbabilityTermStructure> curve(
            ext::make_shared<FlatHazardRate>(
                asofDate,
                Handle<Quote>(ext::make_shared<SimpleQuote>(lambda[i])), dc));
        pUncond.push_back(curve->defaultProbability(testDate));
        std::vector<Issuer::key_curve_pair> curves(1,
            std::make_pair(NorthAmericaCorpDefaultKey(
                EURCurrency(), SeniorSec, Period(), 1.), curve));
        thePool->add(namesIds[i], Issuer(curves), NorthAmericaCorpDefaultKey(
                EURCurrency(), SeniorSec, Period(), 1.));
    }

    ext::shared_ptr<Basket> basket = ext::make_shared<Basket>(
        asofDate, namesIds, std::vector<Real>(names, 10.0), thePool, 0., 1.);

    // uncorrelated names and heterogeneous factor loadings
    std::vector<std::vector<std::vector<Real> > > factorWeights = {
        std::vector<std::vector<Real> >(names, std::vector<Real>(1, 0.0)),
        {{0.1}, {0.6}, {0.3}, {0.75}, {0.45}, {0.2}, {0.5}}
    };

    Real tolerance = 1.0e-12;
    // the conditional probabilities go through an inverse cumulative
    // normal accurate to about 1e-9 relative
    Real independentTolerance = 1.0e-8;

    for (Size k = 0; k < factorWeights.size(); k++) {
        auto model = ext::make_shared<ConstantLossModel<GaussianCopulaPolicy> >(
            factorWeights[k], std::vector<Real>(names, 0.4),
            LatentModelIntegrationType::GaussianQuadrature,
            GaussianCopulaPolicy::initTraits());
        basket->setLossModel(model);

        for (Size n = 0; n <= names + 1; n++) {
            Real calculated = basket->probAtLeastNEvents(n, testDate);

            Real expected = model->integratedExpectedValue(
                [&](const std::vector<Real>& m) {
                    std::vector<Real> pCond(names);
                    for (Size i = 0; i < names; i++)
                        pCond[i] =
                            model->conditionalDefaultProbability(pUncond[i], i, m);
                    return atLeastNEvents(pCond, n);
                });
            if (std::fabs(calculated - expected) > tolerance)
                BOOST_ERROR("failed to reproduce probability of at least "
                            << n << " defaults out of " << names
                            << " (factor loadings set " << k << "):"
                            << std::setprecision(16)
                            << "\n    calculated: " << calculated
                            << "\n    expected:   " << expected);

            if (k == 0) {
                Real independent = atLeastNEvents(pUncond, n);
                if (std::fabs(calculated - independent) > independentTolerance)
                    BOOST_ERROR("failed to reproduce probability of at least "
                                << n << " independent defaults out of " << names << ":"
                                << std::setprecision(16)
                                << "\n    calculated: " << calculated
                                << "\n    expected:   " << independent);
            }
        }
    }
}

#endif

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
