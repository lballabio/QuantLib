/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Sebastian Schlenkrich

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

#include "basismodels.hpp"
#include "utilities.hpp"
#include <ql/compounding.hpp>
#include <ql/experimental/basismodels/swaptioncfs.hpp>
#include <ql/experimental/basismodels/tenoroptionletvts.hpp>
#include <ql/experimental/basismodels/tenorswaptionvts.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionlet.hpp>
#include <ql/termstructures/volatility/optionlet/strippedoptionletadapter.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    // auxiliary data
    Period termsData[] = {
        Period(0, Days),   Period(1, Years), Period(2, Years),  Period(3, Years),
        Period(5, Years),  Period(7, Years), Period(10, Years), Period(15, Years),
        Period(20, Years), Period(61, Years) // avoid extrapolation issues with 30y caplets
    };
    std::vector<Period> terms(termsData, termsData + 10);

    Real discRatesData[] = {-0.00147407, -0.001761684, -0.001736745, -0.00119244, 0.000896055,
                            0.003537077, 0.007213824,  0.011391278,  0.013334611, 0.013982809};
    std::vector<Real> discRates(discRatesData, discRatesData + 10);

    Real proj3mRatesData[] = {-0.000483439, -0.000578569, -0.000383832, 0.000272656, 0.002478699,
                              0.005100113,  0.008750643,  0.012788095,  0.014534052, 0.014942896};
    std::vector<Real> proj3mRates(proj3mRatesData, proj3mRatesData + 10);

    Real proj6mRatesData[] = {0.000233608, 0.000218862, 0.000504018, 0.001240556, 0.003554415,
                              0.006153921, 0.009688264, 0.013521628, 0.015136391, 0.015377704};
    std::vector<Real> proj6mRates(proj6mRatesData, proj6mRatesData + 10);

    Handle<YieldTermStructure> getYTS(const std::vector<Period>& terms,
                                      const std::vector<Real>& rates,
                                      const Real spread = 0.0) {
        Date today = Settings::instance().evaluationDate();
        std::vector<Date> dates;
        for (Size k = 0; k < terms.size(); ++k)
            dates.push_back(NullCalendar().advance(today, terms[k], Unadjusted));
        std::vector<Real> ratesPlusSpread(rates);
        for (Size k = 0; k < ratesPlusSpread.size(); ++k)
            ratesPlusSpread[k] += spread;
        ext::shared_ptr<YieldTermStructure> ts =
            ext::shared_ptr<YieldTermStructure>(new InterpolatedZeroCurve<Cubic>(
                dates, ratesPlusSpread, Actual365Fixed(), NullCalendar()));
        return RelinkableHandle<YieldTermStructure>(ts);
    }

    Period capletTermsData[] = {Period(1, Years),  Period(2, Years),  Period(3, Years),
                                Period(5, Years),  Period(7, Years),  Period(10, Years),
                                Period(15, Years), Period(20, Years), Period(25, Years),
                                Period(30, Years)};
    std::vector<Period> capletTerms(capletTermsData, capletTermsData + 10);

    Real capletStrikesData[] = {-0.0050, 0.0000, 0.0050, 0.0100, 0.0150, 0.0200, 0.0300, 0.0500};
    std::vector<Real> capletStrikes(capletStrikesData, capletStrikesData + 8);

    Real cplRow01[] = {0.003010094, 0.002628065, 0.00456118,  0.006731268,
                       0.008678572, 0.010570881, 0.014149552, 0.021000638};
    Real cplRow02[] = {0.004173715, 0.003727039, 0.004180263, 0.005726083,
                       0.006905876, 0.008263514, 0.010555395, 0.014976523};
    Real cplRow03[] = {0.005870143, 0.005334526, 0.005599775, 0.006633987,
                       0.007773317, 0.009036581, 0.011474391, 0.016277549};
    Real cplRow04[] = {0.007458597, 0.007207522, 0.007263995, 0.007308727,
                       0.007813586, 0.008274858, 0.009743988, 0.012555171};
    Real cplRow05[] = {0.007711531, 0.007608826, 0.007572816, 0.007684107,
                       0.007971932, 0.008283118, 0.009268828, 0.011574083};
    Real cplRow06[] = {0.007619605, 0.007639059, 0.007719825, 0.007823373,
                       0.00800813,  0.008113384, 0.008616374, 0.009785436};
    Real cplRow07[] = {0.007312199, 0.007352993, 0.007369116, 0.007468333,
                       0.007515657, 0.00767695,  0.008020447, 0.009072769};
    Real cplRow08[] = {0.006905851, 0.006966315, 0.007056413, 0.007116494,
                       0.007259661, 0.00733308,  0.007667563, 0.008419696};
    Real cplRow09[] = {0.006529553, 0.006630731, 0.006749022, 0.006858027,
                       0.007001959, 0.007139097, 0.007390404, 0.008036255};
    Real cplRow10[] = {0.006225482, 0.006404012, 0.00651594,  0.006642273,
                       0.006640887, 0.006885713, 0.007093024, 0.00767373};


    Handle<OptionletVolatilityStructure> getOptionletTS() {
        Date today = Settings::instance().evaluationDate();
        std::vector<Date> dates;
        for (Size k = 0; k < capletTerms.size(); ++k)
            dates.push_back(TARGET().advance(today, capletTerms[k], Following));
        // set up vol data manually
        std::vector<std::vector<Real> > capletVols;
        capletVols.push_back(std::vector<Real>(cplRow01, cplRow01 + 8));
        capletVols.push_back(std::vector<Real>(cplRow02, cplRow02 + 8));
        capletVols.push_back(std::vector<Real>(cplRow03, cplRow03 + 8));
        capletVols.push_back(std::vector<Real>(cplRow04, cplRow04 + 8));
        capletVols.push_back(std::vector<Real>(cplRow05, cplRow05 + 8));
        capletVols.push_back(std::vector<Real>(cplRow06, cplRow06 + 8));
        capletVols.push_back(std::vector<Real>(cplRow07, cplRow07 + 8));
        capletVols.push_back(std::vector<Real>(cplRow08, cplRow08 + 8));
        capletVols.push_back(std::vector<Real>(cplRow09, cplRow09 + 8));
        capletVols.push_back(std::vector<Real>(cplRow10, cplRow10 + 8));
        // create quotes
        std::vector<std::vector<Handle<Quote> > > capletVolQuotes;
        for (Size i = 0; i < capletVols.size(); ++i) {
            std::vector<Handle<Quote> > row;
            for (Size j = 0; j < capletVols[i].size(); ++j)
                row.push_back(RelinkableHandle<Quote>(
                    ext::shared_ptr<Quote>(new SimpleQuote(capletVols[i][j]))));
            capletVolQuotes.push_back(row);
        }
        Handle<YieldTermStructure> curve3m = getYTS(terms, proj3mRates);
        ext::shared_ptr<IborIndex> index(new Euribor3M(curve3m));
        ext::shared_ptr<StrippedOptionletBase> tmp1(
            new StrippedOptionlet(2, TARGET(), Following, index, dates, capletStrikes,
                                  capletVolQuotes, Actual365Fixed(), Normal, 0.0));
        ext::shared_ptr<StrippedOptionletAdapter> tmp2(new StrippedOptionletAdapter(tmp1));
        return RelinkableHandle<OptionletVolatilityStructure>(tmp2);
    }

    Period swaptionVTSTermsData[] = {
        Period(1, Years), Period(5, Years), Period(10, Years), Period(20, Years), Period(30, Years),
    };
    std::vector<Period> swaptionVTSTerms(swaptionVTSTermsData, swaptionVTSTermsData + 5);

    Real swtRow01[] = {0.002616, 0.00468, 0.0056, 0.005852, 0.005823};
    Real swtRow02[] = {0.006213, 0.00643, 0.006622, 0.006124, 0.005958};
    Real swtRow03[] = {0.006658, 0.006723, 0.006602, 0.005802, 0.005464};
    Real swtRow04[] = {0.005728, 0.005814, 0.005663, 0.004689, 0.004276};
    Real swtRow05[] = {0.005041, 0.005059, 0.004746, 0.003927, 0.003608};

    Handle<SwaptionVolatilityStructure> getSwaptionVTS() {
        std::vector<std::vector<Real> > swaptionVols;
        swaptionVols.push_back(std::vector<Real>(swtRow01, swtRow01 + 5));
        swaptionVols.push_back(std::vector<Real>(swtRow02, swtRow02 + 5));
        swaptionVols.push_back(std::vector<Real>(swtRow03, swtRow03 + 5));
        swaptionVols.push_back(std::vector<Real>(swtRow04, swtRow04 + 5));
        swaptionVols.push_back(std::vector<Real>(swtRow05, swtRow05 + 5));
        std::vector<std::vector<Handle<Quote> > > swaptionVolQuotes;
        for (Size i = 0; i < swaptionVols.size(); ++i) {
            std::vector<Handle<Quote> > row;
            for (Size j = 0; j < swaptionVols[i].size(); ++j)
                row.push_back(RelinkableHandle<Quote>(
                    ext::shared_ptr<Quote>(new SimpleQuote(swaptionVols[i][j]))));
            swaptionVolQuotes.push_back(row);
        }
        ext::shared_ptr<SwaptionVolatilityStructure> tmp(
            new SwaptionVolatilityMatrix(TARGET(), Following, swaptionVTSTerms, swaptionVTSTerms,
                                         swaptionVolQuotes, Actual365Fixed(), true, Normal));
        return RelinkableHandle<SwaptionVolatilityStructure>(tmp);
    }

    void testSwaptioncfs(bool contTenorSpread) {
        // market data and floating rate index
        Handle<YieldTermStructure> discYTS = getYTS(terms, discRates);
        Handle<YieldTermStructure> proj6mYTS = getYTS(terms, proj6mRates);
        ext::shared_ptr<IborIndex> euribor6m(new Euribor6M(proj6mYTS));
        // Vanilla swap details
        Date today = Settings::instance().evaluationDate();
        Date swapStart = TARGET().advance(today, Period(5, Years), Following);
        Date swapEnd = TARGET().advance(swapStart, Period(10, Years), Following);
        Date exerciseDate = TARGET().advance(swapStart, Period(-2, Days), Preceding);
        Schedule fixedSchedule(swapStart, swapEnd, Period(1, Years), TARGET(), ModifiedFollowing,
                               ModifiedFollowing, DateGeneration::Backward, false);
        Schedule floatSchedule(swapStart, swapEnd, Period(6, Months), TARGET(), ModifiedFollowing,
                               ModifiedFollowing, DateGeneration::Backward, false);
        ext::shared_ptr<VanillaSwap> swap(
            new VanillaSwap(VanillaSwap::Payer, 10000.0, fixedSchedule, 0.03, Thirty360(),
                            floatSchedule, euribor6m, 0.0, euribor6m->dayCounter()));
        swap->setPricingEngine(ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(discYTS)));
        // European exercise and swaption
        ext::shared_ptr<Exercise> europeanExercise(new EuropeanExercise(exerciseDate));
        ext::shared_ptr<Swaption> swaption(
            new Swaption(swap, europeanExercise, Settlement::Physical));
        // calculate basis model swaption cash flows, discount and conmpare with swap
        SwaptionCashFlows cashFlows(swaption, discYTS, contTenorSpread);
        // model time is always Act365Fixed
        Time exerciseTime = Actual365Fixed().yearFraction(discYTS->referenceDate(),
                                                          swaption->exercise()->dates()[0]);
        if (exerciseTime != cashFlows.exerciseTimes()[0])
            BOOST_ERROR(
                "Swaption cash flow exercise time does not coincide with manual calculation");
        // there might be rounding errors
        Real tol = 1.0e-8;
        // (discounted) fixed leg coupons must match swap fixed leg NPV
        Real fixedLeg = 0.0;
        for (Size k = 0; k < cashFlows.fixedTimes().size(); ++k)
            fixedLeg += cashFlows.fixedWeights()[k] * discYTS->discount(cashFlows.fixedTimes()[k]);
        if (fabs(fixedLeg - (-swap->fixedLegNPV())) > tol) // note, '-1' because payer swap
            BOOST_ERROR("Swaption cash flow fixed leg NPV does not match Vanillaswap fixed leg NPV"
                        << "SwaptionCashFlows: " << fixedLeg << "\n"
                        << "swap->fixedLegNPV: " << swap->fixedLegNPV() << "\n"
                        << "Variance:          " << swap->fixedLegNPV() - fixedLeg << "\n");
        // (discounted) floating leg coupons must match swap floating leg NPV
        Real floatLeg = 0.0;
        for (Size k = 0; k < cashFlows.floatTimes().size(); ++k)
            floatLeg += cashFlows.floatWeights()[k] * discYTS->discount(cashFlows.floatTimes()[k]);
        if (fabs(floatLeg - swap->floatingLegNPV()) > tol)
            BOOST_ERROR(
                "Swaption cash flow floating leg NPV does not match Vanillaswap floating leg NPV.\n"
                << "SwaptionCashFlows:    " << floatLeg << "\n"
                << "swap->floatingLegNPV: " << swap->floatingLegNPV() << "\n"
                << "Variance:             " << swap->floatingLegNPV() - floatLeg << "\n");
        // There should not be spread coupons in a single-curve setting.
        // However, if indexed coupons are used the floating leg is not at par,
        // so we need to relax the tolerance to a level at which it will only
        // catch large errors.
        #if defined(QL_USE_INDEXED_COUPON)
        Real tol2 = 0.02;
        #else
        Real tol2 = tol;
        #endif
        SwaptionCashFlows singleCurveCashFlows(swaption, proj6mYTS, contTenorSpread);
        for (Size k = 1; k < singleCurveCashFlows.floatWeights().size() - 1; ++k) {
            if (fabs(singleCurveCashFlows.floatWeights()[k]) > tol2)
                BOOST_ERROR("Swaption cash flow floating leg spread does not vanish in "
                            "single-curve setting.\n"
                            << "Cash flow index k: " << k << ", floatWeights: "
                            << singleCurveCashFlows.floatWeights()[k] << "\n");
        }
    }

}


void BasismodelsTest::testSwaptioncfsContCompSpread() {
    BOOST_TEST_MESSAGE(
        "Testing deterministic tenor basis model with continuous compounded spreads...");
    testSwaptioncfs(true);
}

void BasismodelsTest::testSwaptioncfsSimpleCompSpread() {
    BOOST_TEST_MESSAGE("Testing deterministic tenor basis model with simple compounded spreads...");
    testSwaptioncfs(false);
}

void BasismodelsTest::testTenoroptionletvts() {
    BOOST_TEST_MESSAGE("Testing volatility transformation for caplets/floorlets...");
    // market data and floating rate index
    Real spread = 0.01;
    Handle<YieldTermStructure> discYTS = getYTS(terms, discRates);
    Handle<YieldTermStructure> proj3mYTS = getYTS(terms, proj3mRates);
    Handle<YieldTermStructure> proj6mYTS = getYTS(terms, proj3mRates, spread);
    ext::shared_ptr<IborIndex> euribor3m(new Euribor6M(proj3mYTS));
    ext::shared_ptr<IborIndex> euribor6m(new Euribor6M(proj6mYTS));
    // 3m optionlet VTS
    Handle<OptionletVolatilityStructure> optionletVTS3m = getOptionletTS();
    {
        // we need a correlation structure
        Real corrTimesRaw[] = {0.0, 50.0};
        Real rhoInfDataRaw[] = {0.3, 0.3};
        Real betaDataRaw[] = {0.9, 0.9};
        std::vector<Real> corrTimes(corrTimesRaw, corrTimesRaw + 2);
        std::vector<Real> rhoInfData(rhoInfDataRaw, rhoInfDataRaw + 2);
        std::vector<Real> betaData(betaDataRaw, betaDataRaw + 2);
        ext::shared_ptr<Interpolation> rho(
            new LinearInterpolation(corrTimes.begin(), corrTimes.end(), rhoInfData.begin()));
        ext::shared_ptr<Interpolation> beta(
            new LinearInterpolation(corrTimes.begin(), corrTimes.end(), betaData.begin()));
        ext::shared_ptr<TenorOptionletVTS::CorrelationStructure> corr(
            new TenorOptionletVTS::TwoParameterCorrelation(rho, beta));
        // now we can set up the new volTS and calculate volatilities
        ext::shared_ptr<OptionletVolatilityStructure> optionletVTS6m(
            new TenorOptionletVTS(optionletVTS3m, euribor3m, euribor6m, corr));
        for (Size i = 0; i < capletTerms.size(); ++i) {
            for (Size j = 0; j < capletStrikes.size(); ++j) {
                Real vol3m = optionletVTS3m->volatility(capletTerms[i], capletStrikes[j], true);
                Real vol6m = optionletVTS6m->volatility(capletTerms[i], capletStrikes[j], true);
                Real vol6mShifted =
                    optionletVTS6m->volatility(capletTerms[i], capletStrikes[j] + spread, true);
                // De-correlation yields that larger tenor shifted vols are smaller then shorter
                // tenor vols
                if (vol6mShifted - vol3m >
                    0.0001) // we leave 1bp tolerance due to simplified spread calculation
                    BOOST_ERROR("Shifted 6m vol significantly larger then 3m vol at\n"
                                << "expiry term: " << capletTerms[i]
                                << ", strike: " << capletStrikes[j] << "\n"
                                << "vol3m: " << vol3m << ", vol6m: " << vol6m
                                << ", vol6mShifted: " << vol6mShifted << "\n");
            }
        }
    }
    {
        // we need a correlation structure
        Real corrTimesRaw[] = {0.0, 50.0};
        Real rhoInfDataRaw[] = {0.0, 0.0};
        Real betaDataRaw[] = {0.0, 0.0};
        std::vector<Real> corrTimes(corrTimesRaw, corrTimesRaw + 2);
        std::vector<Real> rhoInfData(rhoInfDataRaw, rhoInfDataRaw + 2);
        std::vector<Real> betaData(betaDataRaw, betaDataRaw + 2);
        ext::shared_ptr<Interpolation> rho(
            new LinearInterpolation(corrTimes.begin(), corrTimes.end(), rhoInfData.begin()));
        ext::shared_ptr<Interpolation> beta(
            new LinearInterpolation(corrTimes.begin(), corrTimes.end(), betaData.begin()));
        ext::shared_ptr<TenorOptionletVTS::CorrelationStructure> corr(
            new TenorOptionletVTS::TwoParameterCorrelation(rho, beta));
        // now we can set up the new volTS and calculate volatilities
        ext::shared_ptr<OptionletVolatilityStructure> optionletVTS6m(
            new TenorOptionletVTS(optionletVTS3m, euribor3m, euribor6m, corr));
        for (Size i = 0; i < capletTerms.size(); ++i) {
            for (Size j = 0; j < capletStrikes.size(); ++j) {
                Real vol3m = optionletVTS3m->volatility(capletTerms[i], capletStrikes[j], true);
                Real vol6m = optionletVTS6m->volatility(capletTerms[i], capletStrikes[j], true);
                Real vol6mShifted =
                    optionletVTS6m->volatility(capletTerms[i], capletStrikes[j] + spread, true);
                // for perfect correlation shifted 6m vols should coincide with 3m vols
                Real tol =
                    (i < 3) ? (0.001) :
                              (0.0001); // 10bp tol for smaller tenors and 1bp tol for larger tenors
                if (fabs(vol6mShifted - vol3m) > tol)
                    BOOST_ERROR("Shifted 6m vol does not match 3m vol for perfect correlation at\n"
                                << "expiry term: " << capletTerms[i]
                                << ", strike: " << capletStrikes[j] << "\n"
                                << "vol3m: " << vol3m << ", vol6m: " << vol6m
                                << ", vol6mShifted: " << vol6mShifted << "\n");
            }
        }
    }
}

void BasismodelsTest::testTenorswaptionvts() {
    BOOST_TEST_MESSAGE("Testing volatility transformation for swaptions...");
    // market data and floating rate index
    Real spread = 0.01;
    Handle<YieldTermStructure> discYTS = getYTS(terms, discRates);
    Handle<YieldTermStructure> proj3mYTS = getYTS(terms, proj3mRates);
    Handle<YieldTermStructure> proj6mYTS = getYTS(terms, proj3mRates, spread);
    ext::shared_ptr<IborIndex> euribor3m(new Euribor6M(proj3mYTS));
    ext::shared_ptr<IborIndex> euribor6m(new Euribor6M(proj6mYTS));
    // Euribor6m ATM vols
    Handle<SwaptionVolatilityStructure> euribor6mSwVTS = getSwaptionVTS();
    {
        ext::shared_ptr<TenorSwaptionVTS> euribor3mSwVTS(
            new TenorSwaptionVTS(euribor6mSwVTS, discYTS, euribor6m, euribor3m, Period(1, Years),
                                 Period(1, Years), Thirty360(), Thirty360()));
        // 6m vols should be slightly larger then 3m vols due to basis
        for (Size i = 0; i < swaptionVTSTerms.size(); ++i) {
            for (Size j = 0; j < swaptionVTSTerms.size(); ++j) {
                Real vol6m = euribor6mSwVTS->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                        0.01, true);
                Real vol3m = euribor3mSwVTS->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                        0.01, true);
                if (vol3m > vol6m)
                    BOOST_ERROR("Euribor 6m must be larger than 3m vol at\n"
                                << "expiry term: " << swaptionVTSTerms[i]
                                << ", swap term: " << swaptionVTSTerms[j] << "\n"
                                << "vol3m: " << vol3m << ", vol6m: " << vol6m << "\n");
            }
        }
    }
    {
        ext::shared_ptr<TenorSwaptionVTS> euribor6mSwVTS2(
            new TenorSwaptionVTS(euribor6mSwVTS, discYTS, euribor6m, euribor6m, Period(1, Years),
                                 Period(1, Years), Thirty360(), Thirty360()));
        // 6m vols to 6m vols should yield initiial vols
        for (Size i = 0; i < swaptionVTSTerms.size(); ++i) {
            for (Size j = 0; j < swaptionVTSTerms.size(); ++j) {
                Real vol6m = euribor6mSwVTS->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                        0.01, true);
                Real vol6m2 = euribor6mSwVTS2->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                          0.01, true);
                Real tol = 1.0e-8;
                if (fabs(vol6m2 - vol6m) > tol)
                    BOOST_ERROR("Euribor 6m to 6m vols should not change at\n"
                                << "expiry term: " << swaptionVTSTerms[i]
                                << ", swap term: " << swaptionVTSTerms[j] << "\n"
                                << "vol6m: " << vol6m << ", vol6m2: " << vol6m2
                                << ", variance: " << (vol6m2 - vol6m) << "\n");
            }
        }
    }
    {
        ext::shared_ptr<TenorSwaptionVTS> euribor3mSwVTS(
            new TenorSwaptionVTS(euribor6mSwVTS, discYTS, euribor6m, euribor3m, Period(1, Years),
                                 Period(1, Years), Thirty360(), Thirty360()));
        ext::shared_ptr<TenorSwaptionVTS> euribor6mSwVTS2(new TenorSwaptionVTS(
            RelinkableHandle<SwaptionVolatilityStructure>(euribor3mSwVTS), discYTS, euribor3m,
            euribor6m, Period(1, Years), Period(1, Years), Thirty360(), Thirty360()));
        // 6m vols to 6m vols should yield initiial vols
        for (Size i = 0; i < swaptionVTSTerms.size(); ++i) {
            for (Size j = 0; j < swaptionVTSTerms.size(); ++j) {
                Real vol6m = euribor6mSwVTS->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                        0.01, true);
                Real vol6m2 = euribor6mSwVTS2->volatility(swaptionVTSTerms[i], swaptionVTSTerms[j],
                                                          0.01, true);
                Real tol = 1.0e-8;
                if (fabs(vol6m2 - vol6m) > tol)
                    BOOST_ERROR("Euribor 6m to 3m to 6m vols should not change at\n"
                                << "expiry term: " << swaptionVTSTerms[i]
                                << ", swap term: " << swaptionVTSTerms[j] << "\n"
                                << "vol6m: " << vol6m << ", vol6m2: " << vol6m2
                                << ", variance: " << (vol6m2 - vol6m) << "\n");
            }
        }
    }
}


test_suite* BasismodelsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Basismodels tests");
    suite->add(QUANTLIB_TEST_CASE(&BasismodelsTest::testSwaptioncfsContCompSpread));
    suite->add(QUANTLIB_TEST_CASE(&BasismodelsTest::testSwaptioncfsSimpleCompSpread));
    suite->add(QUANTLIB_TEST_CASE(&BasismodelsTest::testTenoroptionletvts));
    suite->add(QUANTLIB_TEST_CASE(&BasismodelsTest::testTenorswaptionvts));
    return suite;
}
