/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2014 Peter Caspers

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
#include <ql/indexes/swap/euriborswap.hpp>
#include <ql/cashflows/capflooredcoupon.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/lineartsrpricer.hpp>
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

namespace cms_test {

    struct CommonVars {
        // global data
        RelinkableHandle<YieldTermStructure> termStructure;

        ext::shared_ptr<IborIndex> iborIndex;

        Handle<SwaptionVolatilityStructure> atmVol;
        Handle<SwaptionVolatilityStructure> SabrVolCube1;
        Handle<SwaptionVolatilityStructure> SabrVolCube2;

        std::vector<GFunctionFactory::YieldCurveModel> yieldCurveModels;
        std::vector<ext::shared_ptr<CmsCouponPricer> > numericalPricers;
        std::vector<ext::shared_ptr<CmsCouponPricer> > analyticPricers;

        // cleanup
        SavedSettings backup;

        // setup
        CommonVars() {

            Calendar calendar = TARGET();

            Date referenceDate = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = referenceDate;

            termStructure.linkTo(flatRate(referenceDate, 0.05,
                                          Actual365Fixed()));

            // ATM Volatility structure
            std::vector<Period> atmOptionTenors = {1 * Months, 6 * Months, 1 * Years,
                                                   5 * Years,  10 * Years, 30 * Years};

            std::vector<Period> atmSwapTenors = {1 * Years, 5 * Years, 10 * Years, 30 * Years};

            Matrix m(atmOptionTenors.size(), atmSwapTenors.size());
            m[0][0]=0.1300; m[0][1]=0.1560; m[0][2]=0.1390; m[0][3]=0.1220;
            m[1][0]=0.1440; m[1][1]=0.1580; m[1][2]=0.1460; m[1][3]=0.1260;
            m[2][0]=0.1600; m[2][1]=0.1590; m[2][2]=0.1470; m[2][3]=0.1290;
            m[3][0]=0.1640; m[3][1]=0.1470; m[3][2]=0.1370; m[3][3]=0.1220;
            m[4][0]=0.1400; m[4][1]=0.1300; m[4][2]=0.1250; m[4][3]=0.1100;
            m[5][0]=0.1130; m[5][1]=0.1090; m[5][2]=0.1070; m[5][3]=0.0930;

            atmVol = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(calendar,
                                             Following,
                                             atmOptionTenors,
                                             atmSwapTenors,
                                             m,
                                             Actual365Fixed())));

            // Vol cubes
            std::vector<Period> optionTenors = {{1, Years}, {10, Years}, {30, Years}};
            std::vector<Period> swapTenors = {{2, Years}, {10, Years}, {30, Years}};
            std::vector<Spread> strikeSpreads = {-0.020, -0.005, 0.000, 0.005, 0.020};

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
                    volSpreads[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(
                                    new SimpleQuote(volSpreadsMatrix[i][j])));
                }
            }

            iborIndex = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            ext::shared_ptr<SwapIndex> swapIndexBase(new
                EuriborSwapIsdaFixA(10*Years, termStructure));
            ext::shared_ptr<SwapIndex> shortSwapIndexBase(new
                EuriborSwapIsdaFixA(2*Years, termStructure));

            bool vegaWeightedSmileFit = false;

            SabrVolCube2 = Handle<SwaptionVolatilityStructure>(
                ext::make_shared<SwaptionVolCube2>(atmVol,
                                     optionTenors,
                                     swapTenors,
                                     strikeSpreads,
                                     volSpreads,
                                     swapIndexBase,
                                     shortSwapIndexBase,
                                     vegaWeightedSmileFit));
            SabrVolCube2->enableExtrapolation();

            std::vector<std::vector<Handle<Quote> > > guess(nRows);
            for (Size i=0; i<nRows; ++i) {
                guess[i] = std::vector<Handle<Quote> >(4);
                guess[i][0] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.2)));
                guess[i][1] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
                guess[i][2] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.4)));
                guess[i][3] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
            }
            std::vector<bool> isParameterFixed(4, false);
            isParameterFixed[1] = true;

            // FIXME
            bool isAtmCalibrated = false;

            SabrVolCube1 = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolCube1>(new
                    SwaptionVolCube1(atmVol,
                                     optionTenors,
                                     swapTenors,
                                     strikeSpreads,
                                     volSpreads,
                                     swapIndexBase,
                                     shortSwapIndexBase,
                                     vegaWeightedSmileFit,
                                     guess,
                                     isParameterFixed,
                                     isAtmCalibrated)));
            SabrVolCube1->enableExtrapolation();

            yieldCurveModels = {GFunctionFactory::Standard,
                                GFunctionFactory::ExactYield,
                                GFunctionFactory::ParallelShifts,
                                GFunctionFactory::NonParallelShifts,
                                GFunctionFactory::NonParallelShifts};

            Handle<Quote> zeroMeanRev(ext::make_shared<SimpleQuote>(0.0));

            numericalPricers.clear();
            analyticPricers.clear();
            for (Size j = 0; j < yieldCurveModels.size(); ++j) {
                if (j < yieldCurveModels.size() - 1)
                    numericalPricers.push_back(
                        ext::shared_ptr<CmsCouponPricer>(new NumericHaganPricer(
                            atmVol, yieldCurveModels[j], zeroMeanRev)));
                else
                    numericalPricers.push_back(ext::shared_ptr<CmsCouponPricer>(
                        new LinearTsrPricer(atmVol, zeroMeanRev)));

                analyticPricers.push_back(ext::shared_ptr<CmsCouponPricer>(new
                    AnalyticHaganPricer(atmVol, yieldCurveModels[j],
                                        zeroMeanRev)));
            }
        }
    };

}


void CmsTest::testFairRate()  {

    BOOST_TEST_MESSAGE("Testing Hagan-pricer flat-vol equivalence for coupons...");

    using namespace cms_test;

    CommonVars vars;

    ext::shared_ptr<SwapIndex> swapIndex(new SwapIndex("EuriborSwapIsdaFixA",
                                                       10*Years,
                                                       vars.iborIndex->fixingDays(),
                                                       vars.iborIndex->currency(),
                                                       vars.iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       Unadjusted,
                                                       vars.iborIndex->dayCounter(),//??
                                                       vars.iborIndex));
    // FIXME
    //ext::shared_ptr<SwapIndex> swapIndex(new
    //    EuriborSwapIsdaFixA(10*Years, vars.iborIndex->termStructure()));
    Date startDate = vars.termStructure->referenceDate() + 20*Years;
    Date paymentDate = startDate + 1*Years;
    Date endDate = paymentDate;
    Real nominal = 1.0;
    Rate infiniteCap = Null<Real>();
    Rate infiniteFloor = Null<Real>();
    Real gearing = 1.0;
    Spread spread = 0.0;
    CappedFlooredCmsCoupon coupon(paymentDate, nominal,
                                  startDate, endDate,
                                  swapIndex->fixingDays(), swapIndex,
                                  gearing, spread,
                                  infiniteCap, infiniteFloor,
                                  startDate, endDate,
                                  vars.iborIndex->dayCounter());
    for (Size j=0; j<vars.yieldCurveModels.size(); ++j) {
        vars.numericalPricers[j]->setSwaptionVolatility(vars.atmVol);
        coupon.setPricer(vars.numericalPricers[j]);
        Rate rate0 = coupon.rate();

        vars.analyticPricers[j]->setSwaptionVolatility(vars.atmVol);
        coupon.setPricer(vars.analyticPricers[j]);
        Rate rate1 = coupon.rate();

        Spread difference =  std::fabs(rate1-rate0);
        Spread tol = 2.0e-4;
        bool linearTsr = j==vars.yieldCurveModels.size()-1;

        if (difference > tol)
            BOOST_FAIL("\nCoupon payment date: " << paymentDate <<
                       "\nCoupon start date:   " << startDate <<
                       "\nCoupon floor:        " << io::rate(infiniteFloor) <<
                       "\nCoupon gearing:      " << io::rate(gearing) <<
                       "\nCoupon swap index:   " << swapIndex->name() <<
                       "\nCoupon spread:       " << io::rate(spread) <<
                       "\nCoupon cap:          " << io::rate(infiniteCap) <<
                       "\nCoupon DayCounter:   " << vars.iborIndex->dayCounter()<<
                       "\nYieldCurve Model:    " << vars.yieldCurveModels[j] <<
                       "\nNumerical Pricer:    " << io::rate(rate0) <<
                                   (linearTsr ? " (Linear TSR Model)" : "") <<
                       "\nAnalytic Pricer:     " << io::rate(rate1) <<
                       "\ndifference:          " << io::rate(difference) <<
                       "\ntolerance:           " << io::rate(tol));
    }
}

void CmsTest::testCmsSwap() {

    BOOST_TEST_MESSAGE("Testing Hagan-pricer flat-vol equivalence for swaps...");

    using namespace cms_test;

    CommonVars vars;

    ext::shared_ptr<SwapIndex> swapIndex(new SwapIndex("EuriborSwapIsdaFixA",
                                                       10*Years,
                                                       vars.iborIndex->fixingDays(),
                                                       vars.iborIndex->currency(),
                                                       vars.iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       Unadjusted,
                                                       vars.iborIndex->dayCounter(),//??
                                                       vars.iborIndex));
    // FIXME
    //ext::shared_ptr<SwapIndex> swapIndex(new
    //    EuriborSwapIsdaFixA(10*Years, vars.iborIndex->termStructure()));
    Spread spread = 0.0;
    std::vector<Size> swapLengths = {1, 5, 6, 10};
    Size n = swapLengths.size();
    std::vector<ext::shared_ptr<Swap> > cms(n);
    for (Size i=0; i<n; ++i)
        // no cap, floor
        // no gearing, spread
        cms[i] = MakeCms(Period(swapLengths[i], Years),
                         swapIndex,
                         vars.iborIndex, spread,
                         10*Days);

    for (Size j=0; j<vars.yieldCurveModels.size(); ++j) {
        vars.numericalPricers[j]->setSwaptionVolatility(vars.atmVol);
        vars.analyticPricers[j]->setSwaptionVolatility(vars.atmVol);
        for (Size sl=0; sl<n; ++sl) {
            setCouponPricer(cms[sl]->leg(0), vars.numericalPricers[j]);
            Real priceNum = cms[sl]->NPV();
            setCouponPricer(cms[sl]->leg(0), vars.analyticPricers[j]);
            Real priceAn = cms[sl]->NPV();

            Real difference =  std::fabs(priceNum-priceAn);
            Real tol = 2.0e-4;
            bool linearTsr = j==vars.yieldCurveModels.size()-1;
            if (difference > tol)
                BOOST_FAIL("\nLength in Years:  " << swapLengths[sl] <<
                           //"\nfloor:            " << io::rate(infiniteFloor) <<
                           //"\ngearing:          " << io::rate(gearing) <<
                           "\nswap index:       " << swapIndex->name() <<
                           "\nibor index:       " << vars.iborIndex->name() <<
                           "\nspread:           " << io::rate(spread) <<
                           //"\ncap:              " << io::rate(infiniteCap) <<
                           "\nYieldCurve Model: " << vars.yieldCurveModels[j] <<
                           "\nNumerical Pricer: " << io::rate(priceNum) <<
                                   (linearTsr ? " (Linear TSR Model)" : "") <<
                           "\nAnalytic Pricer:  " << io::rate(priceAn) <<
                           "\ndifference:       " << io::rate(difference) <<
                           "\ntolerance:        " << io::rate(tol));
        }
    }

}

void CmsTest::testParity() {

    BOOST_TEST_MESSAGE("Testing put-call parity for capped-floored CMS coupons...");

    using namespace cms_test;

    CommonVars vars;

    std::vector<Handle<SwaptionVolatilityStructure> > swaptionVols = {
                           vars.atmVol, vars.SabrVolCube1, vars.SabrVolCube2};

    ext::shared_ptr<SwapIndex> swapIndex(new
        EuriborSwapIsdaFixA(10*Years,
                            vars.iborIndex->forwardingTermStructure()));
    Date startDate = vars.termStructure->referenceDate() + 20*Years;
    Date paymentDate = startDate + 1*Years;
    Date endDate = paymentDate;
    Real nominal = 1.0;
    Rate infiniteCap = Null<Real>();
    Rate infiniteFloor = Null<Real>();
    Real gearing = 1.0;
    Spread spread = 0.0;
    DiscountFactor discount = vars.termStructure->discount(paymentDate);
    CappedFlooredCmsCoupon swaplet(paymentDate, nominal,
                                   startDate, endDate,
                                   swapIndex->fixingDays(),
                                   swapIndex,
                                   gearing, spread,
                                   infiniteCap, infiniteFloor,
                                   startDate, endDate,
                                   vars.iborIndex->dayCounter());
    for (Rate strike = .02; strike<.12; strike+=0.05) {
        CappedFlooredCmsCoupon   caplet(paymentDate, nominal,
                                        startDate, endDate,
                                        swapIndex->fixingDays(),
                                        swapIndex,
                                        gearing, spread,
                                        strike, infiniteFloor,
                                        startDate, endDate,
                                        vars.iborIndex->dayCounter());
        CappedFlooredCmsCoupon floorlet(paymentDate, nominal,
                                        startDate, endDate,
                                        swapIndex->fixingDays(),
                                        swapIndex,
                                        gearing, spread,
                                        infiniteCap, strike,
                                        startDate, endDate,
                                        vars.iborIndex->dayCounter());

        for (auto& swaptionVol : swaptionVols) {
            for (Size j=0; j<vars.yieldCurveModels.size(); ++j) {
                vars.numericalPricers[j]->setSwaptionVolatility(swaptionVol);
                vars.analyticPricers[j]->setSwaptionVolatility(swaptionVol);
                std::vector<ext::shared_ptr<CmsCouponPricer> > pricers(2);
                pricers[0] = vars.numericalPricers[j];
                pricers[1] = vars.analyticPricers[j];
                for (Size k=0; k<pricers.size(); ++k) {
                    swaplet.setPricer(pricers[k]);
                    caplet.setPricer(pricers[k]);
                    floorlet.setPricer(pricers[k]);
                    Real swapletPrice = swaplet.price(vars.termStructure) +
                                  nominal * swaplet.accrualPeriod() * strike * discount;
                    Real capletPrice = caplet.price(vars.termStructure);
                    Real floorletPrice = floorlet.price(vars.termStructure);
                    Real difference = std::fabs(capletPrice + floorletPrice -
                                                swapletPrice);
                    Real tol = 2.0e-5;
                    bool linearTsr = k==0 && j==vars.yieldCurveModels.size()-1;
                    if(linearTsr)
                        tol = 1.0e-7;
                    if (difference > tol)
                        BOOST_FAIL("\nCoupon payment date: " << paymentDate <<
                                   "\nCoupon start date:   " << startDate <<
                                   "\nCoupon gearing:      " << io::rate(gearing) <<
                                   "\nCoupon swap index:   " << swapIndex->name() <<
                                   "\nCoupon spread:       " << io::rate(spread) <<
                                   "\nstrike:              " << io::rate(strike) <<
                                   "\nCoupon DayCounter:   " << vars.iborIndex->dayCounter() <<
                                   "\nYieldCurve Model:    " << vars.yieldCurveModels[j] <<
                                   (k==0 ? "\nNumerical Pricer" : "\nAnalytic Pricer") <<
                                   (linearTsr ? " (Linear TSR Model)" : "") <<
                                   "\nSwaplet price:       " << io::rate(swapletPrice) <<
                                   "\nCaplet price:        " << io::rate(capletPrice) <<
                                   "\nFloorlet price:      " << io::rate(floorletPrice) <<
                                   "\ndifference:          " << difference <<
                                   "\ntolerance:           " << io::rate(tol));
                }
            }
        }
    }
}

test_suite* CmsTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Cms tests");
    suite->add(QUANTLIB_TEST_CASE(&CmsTest::testFairRate));
    suite->add(QUANTLIB_TEST_CASE(&CmsTest::testCmsSwap));
    suite->add(QUANTLIB_TEST_CASE(&CmsTest::testParity));
    return suite;
}
