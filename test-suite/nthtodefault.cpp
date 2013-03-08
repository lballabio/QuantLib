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

#include "nthtodefault.hpp"
#include "utilities.hpp"
#include <ql/experimental/credit/nthtodefault.hpp>
#include <ql/experimental/credit/onefactorgaussiancopula.hpp>
#include <ql/experimental/credit/onefactorstudentcopula.hpp>
#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/pricingengines/credit/integralcdsengine.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/quotes/simplequote.hpp>
#include <iostream>

using namespace QuantLib;
using namespace std;
using namespace boost;
using namespace boost::unit_test_framework;

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
        { 9, {   0,   2,   0,   0 } },
        {10, {   0,   1,   0,   1 } }
    };

}

void NthToDefaultTest::testGauss() {
    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Gaussian copula...");

    SavedSettings backup;

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
    vector<Real> lambda (names, 0.01);

    Schedule schedule = MakeSchedule().from(Date (1, September, 2006))
                                      .to(Date (1, September, 2011))
                                      .withTenor(3*Months)
                                      .withCalendar(TARGET());

    Date asofDate(31, August, 2006);

    Settings::instance().evaluationDate() = asofDate;

    vector<Date> gridDates;
    gridDates.push_back (asofDate);
    gridDates.push_back (TARGET().advance (asofDate, Period (1, Years)));
    gridDates.push_back (TARGET().advance (asofDate, Period (5, Years)));
    gridDates.push_back (TARGET().advance (asofDate, Period (7, Years)));

    boost::shared_ptr<YieldTermStructure> yieldPtr (
                                   new FlatForward (asofDate, rate, dc, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    vector<Handle<DefaultProbabilityTermStructure> > probabilities;
    Period maxTerm (10, Years);
    for (Size i = 0; i < lambda.size(); i++) {
        Handle<Quote> h(boost::shared_ptr<Quote>(new SimpleQuote(lambda[i])));
        boost::shared_ptr<DefaultProbabilityTermStructure> ptr (
                                         new FlatHazardRate(asofDate, h, dc));
        probabilities.push_back(Handle<DefaultProbabilityTermStructure>(ptr));
    }

    boost::shared_ptr<SimpleQuote> simpleQuote (new SimpleQuote(0.0));
    Handle<Quote> correlationHandle (simpleQuote);

    boost::shared_ptr<OneFactorCopula> gaussianCopula (
                             new OneFactorGaussianCopula (correlationHandle));
    Handle<OneFactorCopula> copula (gaussianCopula);

    vector<Handle<DefaultProbabilityTermStructure> > singleProbability;
    singleProbability.push_back (probabilities[0]);

    CreditDefaultSwap cds (Protection::Seller, 100.0, 0.02,
                           schedule, Following, Actual360());
    cds.setPricingEngine(boost::shared_ptr<PricingEngine>(
                             new IntegralCdsEngine(timeUnit, probabilities[0],
                                                   recovery, yieldHandle)));

    NthToDefault ftd (1, singleProbability, recovery, copula,
                      Protection::Seller, 100.0, schedule, 0.02,
                      Actual360(), true, yieldHandle, timeUnit);

    Real diff, maxDiff = 0;
    for (Real c = 0; c <= 0.8; c += 0.1) {
        simpleQuote->setValue (c);
        diff = 1e4 * cds.fairSpread() - 1e4 * ftd.fairPremium();
        maxDiff = max (maxDiff, fabs (diff));
    }
    Real tinyTolerance = 0.1;
    BOOST_REQUIRE_MESSAGE (maxDiff < tinyTolerance,
                           "tolerance " << tinyTolerance << " exceeded");

    vector<NthToDefault> ntd;
    for (Size i = 1; i <= probabilities.size(); i++)
        ntd.push_back (NthToDefault (i, probabilities, recovery,
                                     copula, Protection::Seller,
                                     100.0, schedule, 0.02, Actual360(),
                                     true, yieldHandle, timeUnit));

    QL_REQUIRE (LENGTH(hwCorrelation) == 3,
                "correlation length does not match");

    maxDiff = 0;
    for (Size j = 0; j < LENGTH(hwCorrelation); j++) {
        simpleQuote->setValue (hwCorrelation[j]);
        for (Size i = 0; i < ntd.size(); i++) {
            QL_REQUIRE (ntd[i].rank() == hwData[i].rank, "rank does not match");
            QL_REQUIRE (LENGTH(hwCorrelation) == LENGTH(hwData[i].spread),
                        "vector length does not match");
            diff = 1e4 * ntd[i].fairPremium() - hwData[i].spread[j];
            maxDiff = max (maxDiff, fabs (diff));
            BOOST_CHECK_MESSAGE (fabs(diff/hwData[i].spread[j]) < relTolerance
                                 || fabs(diff) < absTolerance,
                                 "tolerance " << relTolerance << "|"
                                 << absTolerance << " exceeded");
        }
    }
}


void NthToDefaultTest::testGaussStudent() {
    BOOST_TEST_MESSAGE("Testing nth-to-default against Hull-White values "
                       "with Gaussian and Student copula...");

    SavedSettings backup;

    /*************************
     * Tolerances
     */
    Real relTolerance = 0.015; // relative difference
    Real absTolerance = 1; // absolute difference in bp

    Period timeUnit = 1*Weeks; // required to reach accuracy

    Size names = 10;
    QL_REQUIRE (LENGTH(hwDataDist) == names,
                "hwDataDist length does not match");

    Real rate = 0.05;
    DayCounter dc = Actual365Fixed();
    Compounding cmp = Continuous; // Simple;


    Real recovery = 0.4;
    vector<Real> lambda (names, 0.01);

    Schedule schedule = MakeSchedule().from(Date (1, September, 2006))
                                      .to(Date (1, September, 2011))
                                      .withTenor(3*Months)
                                      .withCalendar(TARGET());

    Date asofDate(31, August, 2006);

    Settings::instance().evaluationDate() = asofDate;

    vector<Date> gridDates;
    gridDates.push_back (asofDate);
    gridDates.push_back (TARGET().advance (asofDate, Period (1, Years)));
    gridDates.push_back (TARGET().advance (asofDate, Period (5, Years)));
    gridDates.push_back (TARGET().advance (asofDate, Period (7, Years)));

    boost::shared_ptr<YieldTermStructure> yieldPtr (new FlatForward (asofDate, rate, dc, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    vector<Handle<DefaultProbabilityTermStructure> > probabilities;
    Period maxTerm (10, Years);
    for (Size i = 0; i < lambda.size(); i++) {
        Handle<Quote> h(boost::shared_ptr<Quote>(new SimpleQuote(lambda[i])));
        boost::shared_ptr<DefaultProbabilityTermStructure> ptr (
                                         new FlatHazardRate(asofDate, h, dc));
        probabilities.push_back(Handle<DefaultProbabilityTermStructure>(ptr));
    }

    boost::shared_ptr<SimpleQuote> simpleQuote (new SimpleQuote(0.3));
    Handle<Quote> correlationHandle (simpleQuote);

    boost::shared_ptr<OneFactorCopula> gaussianCopula (
                             new OneFactorGaussianCopula (correlationHandle));
    boost::shared_ptr<OneFactorCopula> studentCopula (
                        new OneFactorStudentCopula (correlationHandle, 5, 5));
    RelinkableHandle<OneFactorCopula> copula;

    vector<NthToDefault> ntd;
    for (Size i = 1; i <= probabilities.size(); i++)
        ntd.push_back (NthToDefault (i, probabilities, recovery,
                                     copula, Protection::Seller,
                                     100.0, schedule, 0.02, Actual360(),
                                     true, yieldHandle, timeUnit));

    QL_REQUIRE (LENGTH(hwCorrelation) == 3,
                "correlation length does not match");

    Real maxDiff = 0;

    simpleQuote->setValue (0.3);

    copula.linkTo (gaussianCopula);

    for (Size i = 0; i < ntd.size(); i++) {
        QL_REQUIRE (ntd[i].rank() == hwDataDist[i].rank, "rank does not match");

        Real diff = 1e4 * ntd[i].fairPremium() - hwDataDist[i].spread[0];
        maxDiff = max (maxDiff, fabs (diff));
        BOOST_CHECK_MESSAGE (fabs(diff / hwDataDist[i].spread[0]) ||
                             fabs(diff) < absTolerance,
                             "tolerance " << relTolerance << "|"
                             << absTolerance << " exceeded");
    }

    copula.linkTo (studentCopula);

    maxDiff = 0;
    for (Size i = 0; i < ntd.size(); i++) {
        QL_REQUIRE (ntd[i].rank() == hwDataDist[i].rank, "rank does not match");

        Real diff = 1e4 * ntd[i].fairPremium() - hwDataDist[i].spread[3];
        maxDiff = max (maxDiff, fabs (diff));
        BOOST_CHECK_MESSAGE (fabs(diff / hwDataDist[i].spread[3]) ||
                             fabs(diff) < absTolerance,
                             "tolerance " << relTolerance << "|"
                             << absTolerance << " exceeded");
    }
}

test_suite* NthToDefaultTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Nth-to-default tests");
    suite->add(QUANTLIB_TEST_CASE(&NthToDefaultTest::testGauss));
    suite->add(QUANTLIB_TEST_CASE(&NthToDefaultTest::testGaussStudent));
    return suite;
}

