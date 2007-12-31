/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci

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

#include "cms.hpp"
#include "utilities.hpp"
#include <ql/instruments/swap.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swap/euriborswapfixa.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/makecms.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;
using boost::shared_ptr;

QL_BEGIN_TEST_LOCALS(CmsTest)

RelinkableHandle<YieldTermStructure> termStructure_;

shared_ptr<IborIndex> iborIndex_;

Handle<SwaptionVolatilityStructure> atmVol_;
Handle<SwaptionVolatilityStructure> SabrVolCube1_;
Handle<SwaptionVolatilityStructure> SabrVolCube2_;

std::vector<GFunctionFactory::ModelOfYieldCurve> yieldCurveModels_;
std::vector<shared_ptr<CmsCouponPricer> > numericalPricers_;
std::vector<shared_ptr<CmsCouponPricer> > analyticPricers_;

void setup() {

    Calendar calendar = TARGET();

    Date referenceDate = calendar.adjust(Date::todaysDate());
    Settings::instance().evaluationDate() = referenceDate;

    termStructure_.linkTo(flatRate(referenceDate, 0.05, Actual365Fixed()));

    // ATM Volatility structure
    std::vector<Period> atmOptionTenors;
    atmOptionTenors.push_back(Period(1, Months));
    atmOptionTenors.push_back(Period(6, Months));
    atmOptionTenors.push_back(Period(1, Years));
    atmOptionTenors.push_back(Period(5, Years));
    atmOptionTenors.push_back(Period(10, Years));
    atmOptionTenors.push_back(Period(30, Years));

    std::vector<Period> atmSwapTenors;
    atmSwapTenors.push_back(Period(1, Years));
    atmSwapTenors.push_back(Period(5, Years));
    atmSwapTenors.push_back(Period(10, Years));
    atmSwapTenors.push_back(Period(30, Years));

    Matrix atmVolMatrix(atmOptionTenors.size(), atmSwapTenors.size());
    atmVolMatrix[0][0]=0.1300; atmVolMatrix[0][1]=0.1560; atmVolMatrix[0][2]=0.1390; atmVolMatrix[0][3]=0.1220;
    atmVolMatrix[1][0]=0.1440; atmVolMatrix[1][1]=0.1580; atmVolMatrix[1][2]=0.1460; atmVolMatrix[1][3]=0.1260;
    atmVolMatrix[2][0]=0.1600; atmVolMatrix[2][1]=0.1590; atmVolMatrix[2][2]=0.1470; atmVolMatrix[2][3]=0.1290;
    atmVolMatrix[3][0]=0.1640; atmVolMatrix[3][1]=0.1470; atmVolMatrix[3][2]=0.1370; atmVolMatrix[3][3]=0.1220;
    atmVolMatrix[4][0]=0.1400; atmVolMatrix[4][1]=0.1300; atmVolMatrix[4][2]=0.1250; atmVolMatrix[4][3]=0.1100;
    atmVolMatrix[5][0]=0.1130; atmVolMatrix[5][1]=0.1090; atmVolMatrix[5][2]=0.1070; atmVolMatrix[5][3]=0.0930;

    atmVol_ = Handle<SwaptionVolatilityStructure>(
        shared_ptr<SwaptionVolatilityStructure>(new
            SwaptionVolatilityMatrix(calendar,
                                     atmOptionTenors,
                                     atmSwapTenors,
                                     atmVolMatrix,
                                     Actual365Fixed(),
                                     Following)));

    // Vol cubes
    std::vector<Period> optionTenors;
    optionTenors.push_back(Period(1, Years));
    optionTenors.push_back(Period(10, Years));
    optionTenors.push_back(Period(30, Years));

    std::vector<Period> swapTenors;
    swapTenors.push_back(Period(2, Years));
    swapTenors.push_back(Period(10, Years));
    swapTenors.push_back(Period(30, Years));

    std::vector<Spread> strikeSpreads;
    strikeSpreads.push_back(-0.020);
    strikeSpreads.push_back(-0.005);
    strikeSpreads.push_back(+0.000);
    strikeSpreads.push_back(+0.005);
    strikeSpreads.push_back(+0.020);

    Size nRows = optionTenors.size()*swapTenors.size();
    Size nCols = strikeSpreads.size();
    Matrix volSpreadsMatrix(nRows, nCols);
    volSpreadsMatrix[0][0] =  0.0599;
    volSpreadsMatrix[0][1] =  0.0049;
    volSpreadsMatrix[0][2] =  0.0000;
    volSpreadsMatrix[0][3] = -0.0001;
    volSpreadsMatrix[0][4] =  0.0127;

    volSpreadsMatrix[1][0] =  0.0729;
    volSpreadsMatrix[1][1] =  0.0086;
    volSpreadsMatrix[1][2] =  0.0000;
    volSpreadsMatrix[1][3] = -0.0024;
    volSpreadsMatrix[1][4] =  0.0098;

    volSpreadsMatrix[2][0] =  0.0738;
    volSpreadsMatrix[2][1] =  0.0102;
    volSpreadsMatrix[2][2] =  0.0000;
    volSpreadsMatrix[2][3] = -0.0039;
    volSpreadsMatrix[2][4] =  0.0065;

    volSpreadsMatrix[3][0] =  0.0465;
    volSpreadsMatrix[3][1] =  0.0063;
    volSpreadsMatrix[3][2] =  0.0000;
    volSpreadsMatrix[3][3] = -0.0032;
    volSpreadsMatrix[3][4] = -0.0010;

    volSpreadsMatrix[4][0] =  0.0558;
    volSpreadsMatrix[4][1] =  0.0084;
    volSpreadsMatrix[4][2] =  0.0000;
    volSpreadsMatrix[4][3] = -0.0050;
    volSpreadsMatrix[4][4] = -0.0057;

    volSpreadsMatrix[5][0] =  0.0576;
    volSpreadsMatrix[5][1] =  0.0083;
    volSpreadsMatrix[5][2] =  0.0000;
    volSpreadsMatrix[5][3] = -0.0043;
    volSpreadsMatrix[5][4] = -0.0014;

    volSpreadsMatrix[6][0] =  0.0437;
    volSpreadsMatrix[6][1] =  0.0059;
    volSpreadsMatrix[6][2] =  0.0000;
    volSpreadsMatrix[6][3] = -0.0030;
    volSpreadsMatrix[6][4] = -0.0006;

    volSpreadsMatrix[7][0] =  0.0533;
    volSpreadsMatrix[7][1] =  0.0078;
    volSpreadsMatrix[7][2] =  0.0000;
    volSpreadsMatrix[7][3] = -0.0045;
    volSpreadsMatrix[7][4] = -0.0046;

    volSpreadsMatrix[8][0] =  0.0545;
    volSpreadsMatrix[8][1] =  0.0079;
    volSpreadsMatrix[8][2] =  0.0000;
    volSpreadsMatrix[8][3] = -0.0042;
    volSpreadsMatrix[8][4] = -0.0020;

    std::vector<std::vector<Handle<Quote> > > volSpreads(nRows);
    for (Size i=0; i<nRows; ++i){
        volSpreads[i] = std::vector<Handle<Quote> >(nCols);
        for (Size j=0; j<nCols; ++j) {
            volSpreads[i][j] = Handle<Quote>(shared_ptr<Quote>(new
                SimpleQuote(volSpreadsMatrix[i][j])));
        }
    }

    iborIndex_ = shared_ptr<IborIndex>(new Euribor6M(termStructure_));
    shared_ptr<SwapIndex> swapIndexBase(new
        EuriborSwapFixA(10*Years, termStructure_));

    bool vegaWeightedSmileFit = false;

    SabrVolCube2_ = Handle<SwaptionVolatilityStructure>(
        shared_ptr<SwaptionVolCube2>(new
            SwaptionVolCube2(atmVol_,
                             optionTenors,
                             swapTenors,
                             strikeSpreads,
                             volSpreads,
                             swapIndexBase,
                             vegaWeightedSmileFit)));
    SabrVolCube2_->enableExtrapolation();

    std::vector<std::vector<Handle<Quote> > > guess(nRows);
    for (Size i=0; i<nRows; ++i) {
        guess[i] = std::vector<Handle<Quote> >(4);
        guess[i][0] = Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(0.2)));
        guess[i][1] = Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(0.5)));
        guess[i][2] = Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(0.4)));
        guess[i][3] = Handle<Quote>(shared_ptr<Quote>(new SimpleQuote(0.0)));
    }
    std::vector<bool> isParameterFixed(4, false);
    isParameterFixed[1] = true;

    // FIXME
    bool isAtmCalibrated = false;

    SabrVolCube1_ = Handle<SwaptionVolatilityStructure>(
        shared_ptr<SwaptionVolCube1>(new
            SwaptionVolCube1(atmVol_,
                             optionTenors,
                             swapTenors,
                             strikeSpreads,
                             volSpreads,
                             swapIndexBase,
                             vegaWeightedSmileFit,
                             guess,
                             isParameterFixed,
                             isAtmCalibrated)));
    SabrVolCube1_->enableExtrapolation();

    yieldCurveModels_.clear();
    yieldCurveModels_.push_back(GFunctionFactory::Standard);
    yieldCurveModels_.push_back(GFunctionFactory::ExactYield);
    yieldCurveModels_.push_back(GFunctionFactory::ParallelShifts);
    yieldCurveModels_.push_back(GFunctionFactory::NonParallelShifts);

    Handle<Quote> zeroMeanRev(shared_ptr<Quote>(new SimpleQuote(0.0)));

    numericalPricers_.clear();
    analyticPricers_.clear();
    Size m = yieldCurveModels_.size();
    for (Size j=0; j<m; ++j) {
        numericalPricers_.push_back(shared_ptr<CmsCouponPricer>(new
            ConundrumPricerByNumericalIntegration(atmVol_,
                                                  yieldCurveModels_[j],
                                                  zeroMeanRev)));
        analyticPricers_.push_back(shared_ptr<CmsCouponPricer>(new
            ConundrumPricerByBlack(atmVol_,
                                   yieldCurveModels_[j],
                                   zeroMeanRev)));
    }

}


QL_END_TEST_LOCALS(CmsTest)

void CmsTest::testFairRate()  {

    BOOST_MESSAGE("Testing Pricer equivalence when vol is flat (coupon)...");

    SavedSettings backup;

    setup();

    shared_ptr<SwapIndex> swapIndex(new SwapIndex("EuriborSwapFixA",
                                                  10*Years,
                                                  iborIndex_->fixingDays(),
                                                  iborIndex_->currency(),
                                                  iborIndex_->fixingCalendar(),
                                                  1*Years,
                                                  Unadjusted,
                                                  iborIndex_->dayCounter(),//??
                                                  iborIndex_));
    // FIXME
    //shared_ptr<SwapIndex> swapIndex(new
    //    EuriborSwapFixA(10*Years, iborIndex_->termStructure()));
    Date startDate = termStructure_->referenceDate() + 20*Years;
    Date paymentDate = startDate + 1*Years;
    Date endDate = paymentDate;
    Real nominal = 1.0;
    Rate infiniteCap= 100.0;;
    Rate infiniteFloor = 0.0;
    Real gearing = 1.0;
    Spread spread = 0.0;
    CappedFlooredCmsCoupon coupon(paymentDate, nominal,
                                  startDate, endDate,
                                  swapIndex->fixingDays(), swapIndex,
                                  gearing, spread,
                                  infiniteCap, infiniteFloor,
                                  startDate, endDate,
                                  iborIndex_->dayCounter());
    for (Size j=0; j<yieldCurveModels_.size(); ++j) {
        numericalPricers_[j]->setSwaptionVolatility(atmVol_);
        coupon.setPricer(numericalPricers_[j]);
        Rate rate0 = coupon.rate();

        analyticPricers_[j]->setSwaptionVolatility(atmVol_);
        coupon.setPricer(analyticPricers_[j]);
        Rate rate1 = coupon.rate();

        Spread difference =  std::fabs(rate1-rate0);
        Spread tol = 2.0e-4;
        if (difference > tol)
            BOOST_FAIL("\nCoupon payment date: " << paymentDate <<
                       "\nCoupon start date:   " << startDate <<
                       "\nCoupon floor:        " << io::rate(infiniteFloor) <<
                       "\nCoupon gearing:      " << io::rate(gearing) <<
                       "\nCoupon swap index:   " << swapIndex->name() <<
                       "\nCoupon spread:       " << io::rate(spread) <<
                       "\nCoupon cap:          " << io::rate(infiniteCap) <<
                       "\nCoupon DayCounter:   " << iborIndex_->dayCounter()<<
                       "\nYieldCurve Model:    " << yieldCurveModels_[j] <<
                       "\nNumerical Pricer:    " << io::rate(rate0) <<
                       "\nAnalytic Pricer:     " << io::rate(rate1) <<
                       "\ndifference:          " << io::rate(difference) <<
                       "\ntolerance:           " << io::rate(tol));
    }
}

void CmsTest::testCmsSwap() {

    BOOST_MESSAGE("Testing Pricer equivalence when vol is flat (swaps)...");

    SavedSettings backup;

    setup();

    shared_ptr<SwapIndex> swapIndex(new SwapIndex("EuriborSwapFixA",
                                                  10*Years,
                                                  iborIndex_->fixingDays(),
                                                  iborIndex_->currency(),
                                                  iborIndex_->fixingCalendar(),
                                                  1*Years,
                                                  Unadjusted,
                                                  iborIndex_->dayCounter(),//??
                                                  iborIndex_));
    // FIXME
    //shared_ptr<SwapIndex> swapIndex(new
    //    EuriborSwapFixA(10*Years, iborIndex_->termStructure()));
    Spread spread = 0.0;
    std::vector<Size> swapLengths;
    swapLengths.push_back(1);
    swapLengths.push_back(5);
    swapLengths.push_back(6);
    swapLengths.push_back(10);
    Size n = swapLengths.size();
    std::vector<shared_ptr<Swap> > cms(n);
    for (Size i=0; i<n; ++i)
        // no cap, floor
        // no gearing, spread
        cms[i] = MakeCms(Period(swapLengths[i], Years),
                         swapIndex,
                         iborIndex_, spread,
                         10*Days);

    for (Size j=0; j<yieldCurveModels_.size(); ++j) {
        numericalPricers_[j]->setSwaptionVolatility(atmVol_);
        analyticPricers_[j]->setSwaptionVolatility(atmVol_);
        for (Size sl=0; sl<n; ++sl) {
            setCouponPricer(cms[sl]->leg(0), numericalPricers_[j]);
            Real priceNum = cms[sl]->NPV();
            setCouponPricer(cms[sl]->leg(0), analyticPricers_[j]);
            Real priceAn = cms[sl]->NPV();

            Real difference =  std::fabs(priceNum-priceAn);
            Real tol = 1.0e-4;
            if (difference > tol)
                BOOST_FAIL("\nLength in Years:  " << swapLengths[sl] <<
                           //"\nfloor:            " << io::rate(infiniteFloor) <<
                           //"\ngearing:          " << io::rate(gearing) <<
                           "\nswap index:       " << swapIndex->name() <<
                           "\nibor index:       " << iborIndex_->name() <<
                           "\nspread:           " << io::rate(spread) <<
                           //"\ncap:              " << io::rate(infiniteCap) <<
                           "\nYieldCurve Model: " << yieldCurveModels_[j] <<
                           "\nNumerical Pricer: " << io::rate(priceNum) <<
                           "\nAnalytic Pricer:  " << io::rate(priceAn) <<
                           "\ndifference:       " << io::rate(difference) <<
                           "\ntolerance:        " << io::rate(tol));
        }
    }

}

void CmsTest::testParity() {

    BOOST_MESSAGE("Testing put-call parity for capped-floored CMS coupons...");

    SavedSettings backup;

    setup();

    std::vector<Handle<SwaptionVolatilityStructure> > swaptionVols;
    swaptionVols.push_back(atmVol_);
    swaptionVols.push_back(SabrVolCube1_);
    swaptionVols.push_back(SabrVolCube2_);

    shared_ptr<SwapIndex> swapIndex(new
        EuriborSwapFixA(10*Years, iborIndex_->termStructure()));
    Date startDate = termStructure_->referenceDate() + 20*Years;
    Date paymentDate = startDate + 1*Years;
    Date endDate = paymentDate;
    Real nominal = 1.0;
    Rate infiniteCap= 100.0;;
    Rate infiniteFloor = 0.0;
    Real gearing = 1.0;
    Spread spread = 0.0;
    DiscountFactor discount = termStructure_->discount(paymentDate);
    CappedFlooredCmsCoupon swaplet(paymentDate, nominal,
                                   startDate, endDate,
                                   swapIndex->fixingDays(),
                                   swapIndex,
                                   gearing, spread,
                                   infiniteCap, infiniteFloor,
                                   startDate, endDate,
                                   iborIndex_->dayCounter());
    for (Rate strike = .02; strike<.12; strike+=0.05) {
        CappedFlooredCmsCoupon   caplet(paymentDate, nominal,
                                        startDate, endDate,
                                        swapIndex->fixingDays(),
                                        swapIndex,
                                        gearing, spread,
                                        strike, infiniteFloor,
                                        startDate, endDate,
                                        iborIndex_->dayCounter());
        CappedFlooredCmsCoupon floorlet(paymentDate, nominal,
                                        startDate, endDate,
                                        swapIndex->fixingDays(),
                                        swapIndex,
                                        gearing, spread,
                                        infiniteCap, strike,
                                        startDate, endDate,
                                        iborIndex_->dayCounter());
                
        for (Size i=0; i<swaptionVols.size(); ++i) {
            for (Size j=0; j<yieldCurveModels_.size(); ++j) {
                numericalPricers_[j]->setSwaptionVolatility(swaptionVols[i]);
                analyticPricers_[j]->setSwaptionVolatility(swaptionVols[i]);
                std::vector<shared_ptr<CmsCouponPricer> > pricers(2);
                pricers[0] = numericalPricers_[j];
                pricers[1] = analyticPricers_[j];
                for (Size k=0; k<pricers.size(); ++k) {
                    swaplet.setPricer(pricers[k]);
                    caplet.setPricer(pricers[k]);
                    floorlet.setPricer(pricers[k]);
                    Real swapletPrice = swaplet.price(termStructure_) +
                                  swaplet.accrualPeriod() * strike * discount;
                    Real capletPrice = caplet.price(termStructure_);
                    Real floorletPrice = floorlet.price(termStructure_);
                    Real difference = std::fabs(capletPrice + floorletPrice -
                                                swapletPrice);
                    Real tol = 2.0e-5;
                    if (difference > tol)
                        BOOST_FAIL("\nCoupon payment date: " << paymentDate <<
                                   "\nCoupon start date:   " << startDate <<
                                   "\nCoupon gearing:      " << io::rate(gearing) <<
                                   "\nCoupon swap index:   " << swapIndex->name() <<
                                   "\nCoupon spread:       " << io::rate(spread) <<
                                   "\nstrike:              " << io::rate(strike) <<
                                   "\nCoupon DayCounter:   " << iborIndex_->dayCounter() <<
                                   "\nYieldCurve Model:    " << yieldCurveModels_[j] <<
                                   (k==0 ? "\nNumerical Pricer" : "\nAnalytic Pricer") <<
                                   "\nSwaplet price:       " << io::rate(swapletPrice) <<
                                   "\nCaplet price:        " << io::rate(capletPrice) <<
                                   "\nFloorlet price:      " << io::rate(floorletPrice) <<
                                   "\ndifference:          " << io::rate(difference) <<
                                   "\ntolerance:           " << io::rate(tol));
                }
            }
        }
    }
}

test_suite* CmsTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cms tests");
    suite->add(BOOST_TEST_CASE(&CmsTest::testFairRate));
    suite->add(BOOST_TEST_CASE(&CmsTest::testCmsSwap));
    suite->add(BOOST_TEST_CASE(&CmsTest::testParity));
    return suite;
}
