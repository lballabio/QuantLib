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
#include <ql/experimental/credit/pool.hpp>
#include <ql/experimental/credit/integralcdoengine.hpp>
#include <ql/experimental/credit/midpointcdoengine.hpp>
#include <ql/experimental/credit/randomdefaultlatentmodel.hpp>
#include <ql/experimental/credit/inhomogeneouspooldef.hpp>
#include <ql/experimental/credit/homogeneouspooldef.hpp>
#include <ql/experimental/credit/gaussianlhplossmodel.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/credit/flathazardrate.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/currencies/europe.hpp>
#include <iomanip>
#include <iostream>

using namespace QuantLib;
using namespace std;
using namespace boost::unit_test_framework;

#ifndef QL_PATCH_SOLARIS

namespace cdo_test {

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
        { 0.3, -1, -1, { 1487, 472, 203,  7 } },
        // Opening the T, T&G tests too. The convolution is analytical
        //   now so it runs it a time comparable to the gaussian tests and
        //   has enough precission to pass the tests.
        // Below the T models are integrated with a quadrature, even if this
        //   is incorrect the test pass good enough, the quadrature gets to
        //   be worst as the kernel deviates from a normal, this is low 
        //   orders of the T; here 5 is enough, 3 would not be.
        { 0.3, -1,  5, { 1766, 420, 161,  6 } },
        { 0.3,  5, -1, { 1444, 408, 171, 10 } },
        { 0.3,  5,  5, { 1713, 359, 136,  9 } }
    };

    void check(int i, int j, const string& desc, Real found, Real expected,
               Real bpTolerance, Real relativeTolerance) 
    {
        /* Uncomment to display the full show if your debugging:
        std::cout<< "Case: "<< i << " " << j << " " << found << " :: " 
            << expected  <<  " ("<< desc << ") " << std::endl;
        */
        Real absDiff = found - expected;
        Real relDiff = absDiff / expected;
        BOOST_CHECK_MESSAGE (fabs(relDiff) < relativeTolerance ||
                             fabs(absDiff) < bpTolerance,
                             "case " << i << " " << j << " ("<< desc << "): "
                             << found << " vs. " << expected);
    }

}

#endif


void CdoTest::testHW(unsigned dataSet) {
    #ifndef QL_PATCH_SOLARIS

    BOOST_TEST_MESSAGE ("Testing CDO premiums against Hull-White values"
                        " for data set " << dataSet << "...");

    using namespace cdo_test;

    SavedSettings backup;

    Size poolSize = 100;
    Real lambda = 0.01;

    // nBuckets and period determine the computation time
    Size nBuckets = 200;
    // Period period = 1*Months;
    // for MC engines
    Size numSims = 5000;

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

    ext::shared_ptr<YieldTermStructure> yieldPtr(
                                              new FlatForward (asofDate, rate,
                                                               daycount, cmp));
    Handle<YieldTermStructure> yieldHandle (yieldPtr);

    Handle<Quote> hazardRate(ext::shared_ptr<Quote>(new SimpleQuote(lambda)));
    vector<Handle<DefaultProbabilityTermStructure> > basket;
    ext::shared_ptr<DefaultProbabilityTermStructure> ptr (
               new FlatHazardRate (asofDate,
                                   hazardRate,
                                   ActualActual(ActualActual::ISDA)));
    ext::shared_ptr<Pool> pool (new Pool());
    vector<string> names;
    // probability key items
    vector<Issuer> issuers;
    vector<pair<DefaultProbKey,
           Handle<DefaultProbabilityTermStructure> > > probabilities;
    probabilities.emplace_back(
        NorthAmericaCorpDefaultKey(EURCurrency(), SeniorSec, Period(0, Weeks), 10.),
        Handle<DefaultProbabilityTermStructure>(ptr));

    for (Size i=0; i<poolSize; ++i) {
        ostringstream o;
        o << "issuer-" << i;
        names.push_back(o.str());
        basket.emplace_back(ptr);
        issuers.emplace_back(probabilities);
        pool->add(names.back(), issuers.back(), NorthAmericaCorpDefaultKey(
                EURCurrency(), QuantLib::SeniorSec, Period(), 1.));
    }

    ext::shared_ptr<SimpleQuote> correlation (new SimpleQuote(0.0));
    Handle<Quote> hCorrelation (correlation);
    QL_REQUIRE (LENGTH(hwAttachment) == LENGTH(hwDetachment),
                "data length does not match");

    ext::shared_ptr<PricingEngine> midPCDOEngine( new MidPointCDOEngine(
        yieldHandle));
    ext::shared_ptr<PricingEngine> integralCDOEngine( new IntegralCDOEngine(
        yieldHandle));

    const Size i = dataSet;
    correlation->setValue (hwData7[i].correlation);
    QL_REQUIRE (LENGTH(hwAttachment) == LENGTH(hwData7[i].trancheSpread),
                "data length does not match");
    std::vector<ext::shared_ptr<DefaultLossModel> > basketModels;
    std::vector<std::string> modelNames;
    std::vector<Real> relativeToleranceMidp, relativeTolerancePeriod,
        absoluteTolerance;

    if (hwData7[i].nm == -1 && hwData7[i].nz == -1){
        ext::shared_ptr<GaussianConstantLossLM> gaussKtLossLM(new
            GaussianConstantLossLM(hCorrelation,
            std::vector<Real>(poolSize, recovery),
            LatentModelIntegrationType::GaussianQuadrature, poolSize,
            GaussianCopulaPolicy::initTraits()));

        // 1.-Inhomogeneous gaussian
        modelNames.emplace_back("Inhomogeneous gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            IHGaussPoolLossModel(gaussKtLossLM, nBuckets, 5., -5, 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 2.-homogeneous gaussian
        modelNames.emplace_back("Homogeneous gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            HomogGaussPoolLossModel(gaussKtLossLM, nBuckets, 5., -5, 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 3.-random default gaussian
        modelNames.emplace_back("Random default gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>(new
            RandomDefaultLM<GaussianCopulaPolicy>(gaussKtLossLM, numSims)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.07);
        relativeTolerancePeriod.push_back(0.07);
        // SECOND MC
        // gaussian LHP
        modelNames.emplace_back("Gaussian LHP");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>(new
            GaussianLHPLossModel(hCorrelation,
                std::vector<Real>(poolSize, recovery))));
        absoluteTolerance.push_back(10.);
        relativeToleranceMidp.push_back(0.5);
        relativeTolerancePeriod.push_back(0.5);
        // Binomial...
        // Saddle point...
        // Recursive ...
    }
    else if (hwData7[i].nm > 0 && hwData7[i].nz > 0) {
        TCopulaPolicy::initTraits initTG;
        initTG.tOrders.push_back(hwData7[i].nm);
        initTG.tOrders.push_back(hwData7[i].nz);
        ext::shared_ptr<TConstantLossLM> TKtLossLM(new TConstantLossLM(
            hCorrelation, std::vector<Real>(poolSize, recovery),
            LatentModelIntegrationType::GaussianQuadrature,
            poolSize,
            initTG));
        // 1.-inhomogeneous studentT
        modelNames.emplace_back("Inhomogeneous student");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            IHStudentPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 2.-homogeneous student T
        modelNames.emplace_back("Homogeneous student");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            HomogTPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 3.-random default student T
        modelNames.emplace_back("Random default studentT");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>(new
            RandomDefaultLM<TCopulaPolicy>(TKtLossLM, numSims)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.07);
        relativeTolerancePeriod.push_back(0.07);
        // SECOND MC
        // Binomial...
        // Saddle point...
        // Recursive ...
    }
    else if (hwData7[i].nm > 0 && hwData7[i].nz == -1) {
        TCopulaPolicy::initTraits initTG;
        initTG.tOrders.push_back(hwData7[i].nm);
        initTG.tOrders.push_back(45);
        /* T_{55} is pretty close to a gaussian. Probably theres no need to
        be this conservative as the polynomial convolution gets shorter and
        faster as the order decreases.
        */
        ext::shared_ptr<TConstantLossLM> TKtLossLM(new TConstantLossLM(
            hCorrelation,
            std::vector<Real>(poolSize, recovery),
            LatentModelIntegrationType::GaussianQuadrature,
            poolSize,
            initTG));
        // 1.-inhomogeneous
        modelNames.emplace_back("Inhomogeneous student-gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            IHStudentPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 2.-homogeneous
        modelNames.emplace_back("Homogeneous student-gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            HomogTPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 3.-random default
        modelNames.emplace_back("Random default student-gaussian");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>(new
            RandomDefaultLM<TCopulaPolicy>(TKtLossLM, numSims)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.07);
        relativeTolerancePeriod.push_back(0.07);
        // SECOND MC
        // Binomial...
        // Saddle point...
        // Recursive ...
    }
    else if (hwData7[i].nm == -1 && hwData7[i].nz > 0) {
        TCopulaPolicy::initTraits initTG;
        initTG.tOrders.push_back(45);// pretty close to gaussian
        initTG.tOrders.push_back(hwData7[i].nz);
        ext::shared_ptr<TConstantLossLM> TKtLossLM(new TConstantLossLM(
            hCorrelation,
            std::vector<Real>(poolSize, recovery),
            LatentModelIntegrationType::GaussianQuadrature,
            poolSize,
            initTG));
        // 1.-inhomogeneous gaussian
        modelNames.emplace_back("Inhomogeneous gaussian-student");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            IHStudentPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 2.-homogeneous gaussian
        modelNames.emplace_back("Homogeneous gaussian-student");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>( new
            HomogTPoolLossModel(TKtLossLM, nBuckets, 5., -5., 15)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.04);
        relativeTolerancePeriod.push_back(0.04);
        // 3.-random default gaussian
        modelNames.emplace_back("Random default gaussian-student");
        basketModels.push_back(ext::shared_ptr<DefaultLossModel>(new
            RandomDefaultLM<TCopulaPolicy>(TKtLossLM, numSims)));
        absoluteTolerance.push_back(1.);
        relativeToleranceMidp.push_back(0.07);
        relativeTolerancePeriod.push_back(0.07);
        // SECOND MC
        // Binomial...
        // Saddle point...
        // Recursive ...
    }
    else {
        return;
    }

    for (Size j = 0; j < LENGTH(hwAttachment); j ++) {
        ext::shared_ptr<Basket> basketPtr (
            new Basket(asofDate, names, nominals, pool,
                hwAttachment[j], hwDetachment[j]));
        ostringstream trancheId;
        trancheId << "[" << hwAttachment[j] << " , " << hwDetachment[j]
            << "]";
        SyntheticCDO cdoe(basketPtr, Protection::Seller,
                          schedule, 0.0, premium, daycount, Following);

        for(Size im=0; im<basketModels.size(); im++) {

            basketPtr->setLossModel(basketModels[im]);

            cdoe.setPricingEngine(midPCDOEngine);
            check(i, j, modelNames[im]
                +std::string(" with midp integration on ")+trancheId.str(),
                cdoe.fairPremium() * 1e4, hwData7[i].trancheSpread[j],
                absoluteTolerance[im], relativeToleranceMidp[im]);

            cdoe.setPricingEngine(integralCDOEngine);
            check(i, j, modelNames[im]
                +std::string(" with step integration on ")+trancheId.str(),
                cdoe.fairPremium() * 1e4, hwData7[i].trancheSpread[j],
                absoluteTolerance[im], relativeTolerancePeriod[im]);
        }
    }
    #endif
}


test_suite* CdoTest::suite(SpeedLevel speed) {
    auto* suite = BOOST_TEST_SUITE("CDO tests");

    #ifndef QL_PATCH_SOLARIS
    if (speed == Slow) {
        // unrolled to get different test names
        suite->add(QUANTLIB_TEST_CASE([=](){ CdoTest::testHW(0); }));
        suite->add(QUANTLIB_TEST_CASE([=](){ CdoTest::testHW(1); }));
        suite->add(QUANTLIB_TEST_CASE([=](){ CdoTest::testHW(2); }));
        suite->add(QUANTLIB_TEST_CASE([=](){ CdoTest::testHW(3); }));
        suite->add(QUANTLIB_TEST_CASE([=](){ CdoTest::testHW(4); }));
    }
    #endif
    return suite;
}
