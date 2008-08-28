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

#include "cdo.hpp"
#include "utilities.hpp"
#include <ql/experimental/credit/cdo.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/experimental/credit/onefactorgaussiancopula.hpp>
#include <ql/experimental/credit/onefactorstudentcopula.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace std;
using namespace boost::unit_test_framework;

namespace {

    Real hwAttachment[] = { 0.00, 0.03, 0.06, 0.10 };
    Real hwDetachment[] = { 0.03, 0.06, 0.10, 1.00 };

    struct hwDatum {
        Real correlation;
        Integer nm;
        Integer nz;
        Real trancheSpread[4];
    };

    // HW Table 7
    // corr, Nm, Nz, 0-3, 3-6, 6-10, 10-100
    hwDatum hwData7[] = {
        { 0.1, -1, -1, { 2279, 450,  89,  1 } },
        { 0.3, -1, -1, { 1487, 472, 203,  7 } }
        //  { 0.3, -1,  5, { 1766, 420, 161,  6 } },
        //  { 0.3,  5, -1, { 1444, 408, 171, 10 } },
        //  { 0.3,  5,  5, { 1713, 359, 136,  9 } }
    };

}


void CdoTest::testHW() {

    BOOST_MESSAGE ("Testing CDO premiums against Hull-White values...");

    SavedSettings backup;

    Size names = 100;
    Real lambda = 0.01;

    Real absTolerance = 2; // bps
    Real relTolerance = 0.03; // 3%

    // nBuckets and period determine the computation time
    Size nBuckets = 100;
    Period period = 1*Weeks;

    Real rate = 0.05;
    DayCounter daycount = Actual360();
    Compounding cmp = Continuous; // Simple;

    Real recovery = 0.4;
    vector<Real> nominals(names, 100.0);
    Real premium = 0.02;
    Period maxTerm (5, Years);
    Schedule schedule = MakeSchedule (Date (1, September, 2006),
                                      Date (1, September, 2011),
                                      Period (3, Months),
                                      TARGET(), Following);

    Date asofDate = Date(31, August, 2006);

    Settings::instance().evaluationDate() = asofDate;

    boost::shared_ptr<YieldTermStructure> yieldPtr(
                                              new FlatForward (asofDate, rate,
                                                               daycount, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    Handle<Quote> hazardRate(boost::shared_ptr<Quote>(new SimpleQuote(lambda)));
    vector<Handle<DefaultProbabilityTermStructure> > basket;
    boost::shared_ptr<DefaultProbabilityTermStructure> ptr (
               new FlatHazardRate (asofDate,
                                   hazardRate,
                                   Actual365Fixed()));
    for (Size i=0; i<names; ++i) {
        basket.push_back(Handle<DefaultProbabilityTermStructure>(ptr));
    }

    boost::shared_ptr<SimpleQuote> correlation (new SimpleQuote(0.0));
    Handle<Quote> hCorrelation (correlation);

    boost::shared_ptr<OneFactorCopula> pGaussianCopula (
                                  new OneFactorGaussianCopula (hCorrelation));
    RelinkableHandle<OneFactorCopula> hCopula (pGaussianCopula);

    QL_REQUIRE (LENGTH(hwAttachment) == LENGTH(hwDetachment),
                "data length does not match");
    Real absDiff, relDiff, maxDiff = 0, maxRelDiff = 0;
    for (Size i = 0; i < LENGTH(hwData7); i++) {
        correlation->setValue (hwData7[i].correlation);
        QL_REQUIRE (LENGTH(hwAttachment) == LENGTH(hwData7[i].trancheSpread),
                    "data length does not match");
        if (hwData7[i].nm == -1 && hwData7[i].nz == -1)
            hCopula.linkTo (pGaussianCopula);
        else if (hwData7[i].nm > 0 && hwData7[i].nz > 0) {
            boost::shared_ptr<OneFactorCopula> pStudentCopula (
                                  new OneFactorStudentCopula (hCorrelation,
                                                              hwData7[i].nm,
                                                              hwData7[i].nz));
            hCopula.linkTo (pStudentCopula);
        }
        else if (hwData7[i].nm > 0 && hwData7[i].nz == -1) {
            boost::shared_ptr<OneFactorCopula> pSGCopula (
                          new OneFactorStudentGaussianCopula (hCorrelation,
                                                              hwData7[i].nm));
            hCopula.linkTo (pSGCopula);
        }
        else if (hwData7[i].nm == -1 && hwData7[i].nz > 0) {
            boost::shared_ptr<OneFactorCopula> pGSCopula (
                          new OneFactorGaussianStudentCopula (hCorrelation,
                                                              hwData7[i].nz));
            hCopula.linkTo (pGSCopula);
        }
        else {
            continue;
        }

        for (Size j = 0; j < LENGTH(hwAttachment); j ++) {
            CDO cdo (hwAttachment[j], hwDetachment[j],
                     nominals, basket, hCopula,
                     true, schedule, premium, daycount, recovery, 0.0,
                     yieldHandle, nBuckets, period);

            absDiff = cdo.fairPremium() * 1e4 - hwData7[i].trancheSpread[j];
            relDiff = absDiff / hwData7[i].trancheSpread[j];
            maxDiff = max (maxDiff, fabs (absDiff));
            maxRelDiff = max (maxRelDiff, fabs (relDiff));
            ostringstream o;
            o.setf (ios::fixed, ios::floatfield);
            o << setprecision(2) << setw (4) << correlation->value() << " "
              << setw (2) << hwData7[i].nm << " "
              << setw (2) << hwData7[i].nz << " "
              << setprecision(2) << setw(5) << hwAttachment[j] << " "
              << setprecision(2) << setw(5) << hwDetachment[j] << " "
              << setprecision(0) << setw(8) << cdo.fairPremium() * 1e4 << " "
              << setprecision(0) << setw(8) << hwData7[i].trancheSpread[j] << " "
              << setprecision(2) << setw(8) << absDiff << " "
              << setprecision(1) << setw(6) << 100.0 * relDiff << " " << cdo.error();

            BOOST_CHECK_MESSAGE (fabs(relDiff) < relTolerance ||
                                 fabs(absDiff) < absTolerance,
                                 "tolerance " << relTolerance
                                 << "|" << absTolerance
                                 << " exceeded in: " << o.str());
        }
    }
}


test_suite* CdoTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CDO tests");
    suite->add(BOOST_TEST_CASE(&CdoTest::testHW));
    return suite;
}
