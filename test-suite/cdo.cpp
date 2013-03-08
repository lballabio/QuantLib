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
#include <ql/experimental/credit/syntheticcdoengines.hpp>
#include <ql/experimental/credit/onefactorgaussiancopula.hpp>
#include <ql/experimental/credit/onefactorstudentcopula.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/currencies/europe.hpp>
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

    void check(int i, int j, string desc, Real found, Real expected,
               Real bpTolerance, Real relativeTolerance) {
        Real absDiff = found - expected;
        Real relDiff = absDiff / expected;
        BOOST_CHECK_MESSAGE (fabs(relDiff) < relativeTolerance ||
                             fabs(absDiff) < bpTolerance,
                             "case " << i << " " << j << " ("<< desc << "): "
                             << found << " vs. " << expected);
    }

}

void CdoTest::testHW() {

    BOOST_TEST_MESSAGE ("Testing CDO premiums against Hull-White values...");

    SavedSettings backup;

    Size poolSize = 100;
    Real lambda = 0.01;

    // nBuckets and period determine the computation time
    Size nBuckets = 200;
    Period period = 1*Months;

    Real rate = 0.05;
    DayCounter daycount = Actual360();
    Compounding cmp = Continuous; // Simple;

    Real recovery = 0.4;
    vector<Real> nominals(poolSize, 100.0);
    Real premium = 0.02;
    Period maxTerm (5, Years);
    Schedule schedule = MakeSchedule().from(Date (1, September, 2006))
                                      .to(Date (1, September, 2011))
                                      .withTenor(Period (3, Months))
                                      .withCalendar(TARGET());

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
                                   ActualActual()));
    boost::shared_ptr<Pool> pool (new Pool());
    vector<string> names;
    // probability key items
    vector<Issuer> issuers;
    vector<pair<DefaultProbKey,
           Handle<DefaultProbabilityTermStructure> > > probabilities;
    probabilities.push_back(std::make_pair(
        NorthAmericaCorpDefaultKey(EURCurrency(),
                                   SeniorSec,
                                   Period(0,Weeks),
                                   10.),
       Handle<DefaultProbabilityTermStructure>(ptr)));

    for (Size i=0; i<poolSize; ++i) {
        ostringstream o;
        o << "issuer-" << i;
        names.push_back(o.str());
        basket.push_back(Handle<DefaultProbabilityTermStructure>(ptr));
        issuers.push_back(Issuer(probabilities));
        pool->add(names.back(), issuers.back());
    }

    boost::shared_ptr<SimpleQuote> correlation (new SimpleQuote(0.0));
    Handle<Quote> hCorrelation (correlation);

    boost::shared_ptr<OneFactorCopula> pGaussianCopula (
                            new OneFactorGaussianCopula (hCorrelation));
    RelinkableHandle<OneFactorCopula> hCopula (pGaussianCopula);

    boost::shared_ptr<RandomDefaultModel> rdm(new
        GaussianRandomDefaultModel(pool,
                                   std::vector<DefaultProbKey>(poolSize,
                                    NorthAmericaCorpDefaultKey(EURCurrency(),
                                                               SeniorSec)),
                                   hCopula,
                                   1.e-6,
                                   42));

    boost::shared_ptr<PricingEngine> engine1(
                          new IHPIntegralCDOEngine(hCopula, nBuckets));
    boost::shared_ptr<PricingEngine> engine2(
                          new IHPMidPointCDOEngine(hCopula, nBuckets));
    boost::shared_ptr<PricingEngine> engine3(
                          new HPIntegralCDOEngine(hCopula, nBuckets));
    boost::shared_ptr<PricingEngine> engine4(
                          new HPMidPointCDOEngine(hCopula, nBuckets));
    boost::shared_ptr<PricingEngine> engine5(
                          new MonteCarloCDOEngine1(rdm, 10000));
    boost::shared_ptr<PricingEngine> engine6(
                          new MonteCarloCDOEngine2(rdm, 10000));
    boost::shared_ptr<PricingEngine> engine7(
                          new GLHPMidPointCDOEngine(hCopula));

    QL_REQUIRE (LENGTH(hwAttachment) == LENGTH(hwDetachment),
                "data length does not match");

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
            boost::shared_ptr<Basket> basketPtr (
                new Basket(names,
                           nominals,
                           pool,
                           std::vector<DefaultProbKey>(
                                names.size(),
                                NorthAmericaCorpDefaultKey(EURCurrency(),
                                                           SeniorSec)),
                           std::vector<boost::shared_ptr<RecoveryRateModel> > (
                                names.size(),
                                boost::shared_ptr<RecoveryRateModel>(
                                    new ConstantRecoveryModel(recovery,
                                                              SeniorSec))),
                           hwAttachment[j],
                           hwDetachment[j]));

            CDO cdo (hwAttachment[j], hwDetachment[j],
                     nominals, basket, hCopula,
                     true, schedule, premium, daycount, recovery, 0.0,
                     yieldHandle, nBuckets, period);

            SyntheticCDO cdoe(basketPtr, Protection::Seller,
                              schedule, 0.0, premium, daycount, Following,
                              yieldHandle);

            check(i, j, "performCalculations", cdo.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.02);

            cdoe.setPricingEngine(engine1);
            check(i, j, "IHPIntegralEngine", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.03);

            cdoe.setPricingEngine(engine2);
            check(i, j, "IHPMidPointEngine", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.04);

            cdoe.setPricingEngine(engine3);
            check(i, j, "HPIntegralEngine", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.03);

            cdoe.setPricingEngine(engine4);
            check(i, j, "HPMidPointEngine", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.04);

            cdoe.setPricingEngine(engine5);
            check(i, j, "McEngine1 10k", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.07);

            cdoe.setPricingEngine(engine6);
            check(i, j, "McEngine2 10k", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 1, 0.07);

            cdoe.setPricingEngine(engine7);
            check(i, j, "Gaussian LHP", cdoe.fairPremium() * 1e4,
                  hwData7[i].trancheSpread[j], 10, 0.5);
        }
    }
}


test_suite* CdoTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("CDO tests");
    suite->add(QUANTLIB_TEST_CASE(&CdoTest::testHW));
    return suite;
}
