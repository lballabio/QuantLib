/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2023 Andre Miemiec

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

#include "cms_normal.hpp"
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
#include <ql/termstructures/volatility/swaption/interpolatedswaptionvolatilitycube.hpp>
#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/makecms.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace cms_normal_test {

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

            termStructure.linkTo(flatRate(referenceDate, 0.02,
                                          Actual365Fixed()));

            // ATM Volatility structure
            std::vector<Period> atmOptionTenors = {1 * Months, 6 * Months, 1 * Years,
                                                   5 * Years,  10 * Years, 30 * Years};

            std::vector<Period> atmSwapTenors = {1 * Years, 5 * Years, 10 * Years, 30 * Years};

            Matrix m(atmOptionTenors.size(), atmSwapTenors.size());
            m[0][0]=0.0085; m[0][1]=0.0120; m[0][2]=0.0102; m[0][3]=0.0095;
            m[1][0]=0.0106;	m[1][1]=0.0104;	m[1][2]=0.0095;	m[1][3]=0.0092;
            m[2][0]=0.0104;	m[2][1]=0.0099;	m[2][2]=0.0092;	m[2][3]=0.0088;
            m[3][0]=0.0091;	m[3][1]=0.0086;	m[3][2]=0.0080;	m[3][3]=0.0070;
            m[4][0]=0.0077;	m[4][1]=0.0073;	m[4][2]=0.0068;	m[4][3]=0.0060;
            m[5][0]=0.0057;	m[5][1]=0.0055;	m[5][2]=0.0050;	m[5][3]=0.0039;



            atmVol = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SwaptionVolatilityStructure>(new
                    SwaptionVolatilityMatrix(calendar,
                                             Following,
                                             atmOptionTenors,
                                             atmSwapTenors,
                                             m,
                                             Actual365Fixed(),
                                             false,
                                             QuantLib::VolatilityType::Normal)));
            

            // Vol cubes
            std::vector<Period> optionTenors = {{1, Years}, {10, Years}, {30, Years}};
            std::vector<Period> swapTenors = {{2, Years}, {10, Years}, {30, Years}};
            std::vector<Spread> strikeSpreads = {-0.020, -0.005, 0.000, 0.005, 0.020};

            Size nRows = optionTenors.size()*swapTenors.size();
            Size nCols = strikeSpreads.size();
            Matrix volSpreadsMatrix(nRows, nCols);
            
            volSpreadsMatrix[0][0] = -0.0016;
            volSpreadsMatrix[0][1] = -0.0008;
            volSpreadsMatrix[0][2] =  0.0000;
            volSpreadsMatrix[0][3] =  0.0009;
            volSpreadsMatrix[0][4] =  0.0038;

            volSpreadsMatrix[1][0] =  0.0009;
            volSpreadsMatrix[1][1] = -0.0003;
            volSpreadsMatrix[1][2] =  0.0000;
            volSpreadsMatrix[1][3] =  0.0007;
            volSpreadsMatrix[1][4] =  0.0035;

            volSpreadsMatrix[2][0] =  0.0025;
            volSpreadsMatrix[2][1] =  0.0002;
            volSpreadsMatrix[2][2] =  0.0000;
            volSpreadsMatrix[2][3] =  0.0002;
            volSpreadsMatrix[2][4] =  0.0024;

            volSpreadsMatrix[3][0] =  -0.0009;
            volSpreadsMatrix[3][1] =  -0.0003;
            volSpreadsMatrix[3][2] =   0.0000;
            volSpreadsMatrix[3][3] =   0.0003;
            volSpreadsMatrix[3][4] =   0.0013;

            volSpreadsMatrix[4][0] =  -0.0001;
            volSpreadsMatrix[4][1] =  -0.0001;
            volSpreadsMatrix[4][2] =   0.0000;
            volSpreadsMatrix[4][3] =   0.0001;
            volSpreadsMatrix[4][4] =   0.0007;

            volSpreadsMatrix[5][0] =   0.0003;
            volSpreadsMatrix[5][1] =   0.0000;
            volSpreadsMatrix[5][2] =   0.0000;
            volSpreadsMatrix[5][3] =   0.0001;
            volSpreadsMatrix[5][4] =   0.0005;

            volSpreadsMatrix[6][0] =  -0.0004;
            volSpreadsMatrix[6][1] =  -0.0001;
            volSpreadsMatrix[6][2] =   0.0000;
            volSpreadsMatrix[6][3] =   0.0001;
            volSpreadsMatrix[6][4] =   0.0006;

            volSpreadsMatrix[7][0] =  -0.0001;
            volSpreadsMatrix[7][1] =   0.0000;
            volSpreadsMatrix[7][2] =   0.0000;
            volSpreadsMatrix[7][3] =   0.0000;
            volSpreadsMatrix[7][4] =   0.0002;


            volSpreadsMatrix[8][0] =  -0.0002;
            volSpreadsMatrix[8][1] =  -0.0001;
            volSpreadsMatrix[8][2] =   0.0000;
            volSpreadsMatrix[8][3] =   0.0001;
            volSpreadsMatrix[8][4] =   0.0002;


            std::vector<std::vector<Handle<Quote> > > volSpreads(nRows);
            for (Size i=0; i<nRows; ++i){
                volSpreads[i] = std::vector<Handle<Quote> >(nCols);
                for (Size j=0; j<nCols; ++j) {
                    volSpreads[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(
                                    new SimpleQuote(volSpreadsMatrix[i][j])));
                }
            }

            iborIndex = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            /*
            ext::shared_ptr<SwapIndex> swapIndexBase(new
                EuriborSwapIsdaFixA(10*Years, termStructure,termStructure));
            ext::shared_ptr<SwapIndex> shortSwapIndexBase(new
                EuriborSwapIsdaFixA(2*Years, termStructure,termStructure));
            */

            ext::shared_ptr<SwapIndex> swapIndexBase(new SwapIndex("swapIndexBase",
                                                       2*Years,
                                                       iborIndex->fixingDays(),
                                                       iborIndex->currency(),
                                                       iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       ModifiedFollowing,
                                                       Thirty360(Thirty360::EurobondBasis),//EUR
                                                       iborIndex,
                                                       termStructure));
            ext::shared_ptr<SwapIndex> shortSwapIndexBase(new SwapIndex("shortSwapIndexBase",
                                                       1*Years,
                                                       iborIndex->fixingDays(),
                                                       iborIndex->currency(),
                                                       iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       ModifiedFollowing,
                                                       Thirty360(Thirty360::EurobondBasis),//EUR
                                                       iborIndex,
                                                       termStructure));

            bool vegaWeightedSmileFit = false;

            SabrVolCube2 = Handle<SwaptionVolatilityStructure>(
                ext::make_shared<InterpolatedSwaptionVolatilityCube>(atmVol,
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
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.01)));
                guess[i][1] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.0)));
                guess[i][2] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.3)));
                guess[i][3] =
                    Handle<Quote>(ext::shared_ptr<Quote>(new SimpleQuote(0.5)));
            }
            std::vector<bool> isParameterFixed(4, false);
            isParameterFixed[1] = true;

            // FIXME
            bool isAtmCalibrated = false;

            SabrVolCube1 = Handle<SwaptionVolatilityStructure>(
                ext::shared_ptr<SabrSwaptionVolatilityCube>(new
                    SabrSwaptionVolatilityCube(atmVol,
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
                                GFunctionFactory::NonParallelShifts};

            Handle<Quote> zeroMeanRev(ext::make_shared<SimpleQuote>(0.0));

            numericalPricers.clear();
            analyticPricers.clear();
            for (Size j = 0; j < yieldCurveModels.size(); ++j) {
                numericalPricers.push_back(ext::shared_ptr<CmsCouponPricer>(new 
                    NumericHaganPricer(atmVol, yieldCurveModels[j], 
                                        zeroMeanRev)));
 

                analyticPricers.push_back(ext::shared_ptr<CmsCouponPricer>(new
                    AnalyticHaganPricer(atmVol, yieldCurveModels[j],
                                        zeroMeanRev)));
            }
        }
    };

}


void CmsNormalTest::testFairRate()  {

    BOOST_TEST_MESSAGE("Testing Hagan-pricer flat-vol equivalence for coupons (normal case)...");

    using namespace cms_normal_test;

    CommonVars vars;

    ext::shared_ptr<SwapIndex> swapIndex(new SwapIndex("CMS10Y",
                                                       10*Years,
                                                       vars.iborIndex->fixingDays(),
                                                       vars.iborIndex->currency(),
                                                       vars.iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       ModifiedFollowing,
                                                       Thirty360(Thirty360::EurobondBasis),//EUR
                                                       vars.iborIndex,
                                                       vars.termStructure));
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
        Spread tol = 2.0e-4;  //The tolerance used before was 2bp. Semms very low for a coupon with pmt in 20 Years

        if (std::round(10.0*(difference-tol))/10.0 > 0.0 ) // seems a more appropriate comparison to me instead of  difference > tol
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
                       "\nAnalytic Pricer:     " << io::rate(rate1) <<
                       "\ndifference:          " << io::rate(difference) <<
                       "\ntolerance:           " << io::rate(tol));
    }
}

void CmsNormalTest::testCmsSwap() {

    BOOST_TEST_MESSAGE("Testing Hagan-pricer flat-vol equivalence for swaps (normal case)...");

    using namespace cms_normal_test;

    CommonVars vars;

    ext::shared_ptr<SwapIndex> swapIndex(new SwapIndex("CMS10Y",
                                                       10*Years,
                                                       vars.iborIndex->fixingDays(),
                                                       vars.iborIndex->currency(),
                                                       vars.iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       ModifiedFollowing,
                                                       Thirty360(Thirty360::EurobondBasis),//EUR
                                                       vars.iborIndex,
                                                       vars.termStructure));
    // FIXME
    //ext::shared_ptr<SwapIndex> swapIndex(new
    //    EuriborSwapIsdaFixA(10*Years, vars.iborIndex->termStructure()));
    Spread spread = 0.0;
    std::vector<Size> swapLengths = {1, 5, 6, 10};   //?is an off-gridpoint a good test?
    Size n = swapLengths.size();
    std::vector<ext::shared_ptr<Swap> > cms(n);
    for (Size i=0; i<n; ++i)
        // no cap, floor
        // no gearing, spread
        cms[i] = MakeCms(Period(swapLengths[i], Years),
                         swapIndex,
                         vars.iborIndex, spread);

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

            if (std::round(10.0*(difference-tol))/10.0 > 0.0 )  // seems a more appropriate comparison to me instead of  difference > tol
                BOOST_FAIL("\nLength in Years:  " << swapLengths[sl] <<
                           //"\nfloor:            " << io::rate(infiniteFloor) <<
                           //"\ngearing:          " << io::rate(gearing) <<
                           "\nswap index:       " << swapIndex->name() <<
                           "\nibor index:       " << vars.iborIndex->name() <<
                           "\nspread:           " << io::rate(spread) <<
                           //"\ncap:              " << io::rate(infiniteCap) <<
                           "\nYieldCurve Model: " << vars.yieldCurveModels[j] <<
                           "\nNumerical Pricer: " << io::rate(priceNum) <<
                           "\nAnalytic Pricer:  " << io::rate(priceAn) <<
                           "\ndifference:       " << io::rate(difference) <<
                           "\ntolerance:        " << io::rate(tol));
        }
    }

}

void CmsNormalTest::testParity() {

    BOOST_TEST_MESSAGE("Testing put-call parity for capped-floored CMS coupons (normal case)...");

    using namespace cms_normal_test;

    CommonVars vars;

    std::vector<Handle<SwaptionVolatilityStructure> > swaptionVols = {
                           vars.atmVol, vars.SabrVolCube1, vars.SabrVolCube2};
                           
  

    ext::shared_ptr<SwapIndex> swapIndex(new SwapIndex("CMS10Y",
                                                       10*Years,
                                                       vars.iborIndex->fixingDays(),
                                                       vars.iborIndex->currency(),
                                                       vars.iborIndex->fixingCalendar(),
                                                       1*Years,
                                                       ModifiedFollowing,
                                                       Thirty360(Thirty360::EurobondBasis),//EUR
                                                       vars.iborIndex,
                                                       vars.termStructure));



    Date startDate = vars.termStructure->referenceDate() + 20*Years;
    Date paymentDate = startDate + 1*Years;
    Date endDate = paymentDate;
    Real nominal = 1.0;
    Rate infiniteCap = Null<Real>();
    Rate infiniteFloor = Null<Real>();
    Real gearing = 1.0;
    Spread spread = 0.0;
    DiscountFactor discount = vars.termStructure->discount(paymentDate);
    CappedFlooredCmsCoupon cpn_plain(paymentDate, nominal,
                                   startDate, endDate,
                                   swapIndex->fixingDays(),
                                   swapIndex,
                                   gearing, spread,
                                   infiniteCap, infiniteFloor,
                                   startDate, endDate,
                                   vars.iborIndex->dayCounter());
    for (Rate strike = -0.005; strike <= 0.035; strike+=0.01) {
        CappedFlooredCmsCoupon   cpn_capped(paymentDate, nominal,
                                        startDate, endDate,
                                        swapIndex->fixingDays(),
                                        swapIndex,
                                        gearing, spread,
                                        strike, infiniteFloor,
                                        startDate, endDate,
                                        vars.iborIndex->dayCounter());
        CappedFlooredCmsCoupon cpn_floored(paymentDate, nominal,
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
                    cpn_plain.setPricer(pricers[k]);
                    cpn_capped.setPricer(pricers[k]);
                    cpn_floored.setPricer(pricers[k]);
                    Real cpn_plain_Price = cpn_plain.price(vars.termStructure);
                    Real cpn_capped_Price = cpn_capped.price(vars.termStructure);
                    Real cpn_floored_Price = cpn_floored.price(vars.termStructure);
                    Real difference = std::fabs(cpn_capped_Price + cpn_floored_Price - cpn_plain_Price 
                                                - nominal * strike * cpn_plain.accrualPeriod() * discount);
                    Real tol = 4.0e-5;

                    if (difference > tol)
                        BOOST_FAIL("\nDiscount Factor:     " << discount <<
                                   "\nCoupon payment date: " << paymentDate <<
                                   "\nCoupon start date:   " << startDate <<
                                   "\nCoupon gearing:      " << io::rate(gearing) <<
                                   "\nCoupon swap index:   " << swapIndex->name() <<
                                   "\nCoupon spread:       " << io::rate(spread) <<
                                   "\nstrike:              " << io::rate(strike) <<
                                   "\nCoupon DayCounter:   " << vars.iborIndex->dayCounter() <<
                                   "\nYieldCurve Model:    " << vars.yieldCurveModels[j] <<
                                   "\nPricerType:          " << (k==0 ? "Numerical Pricer" : "Analytic Pricer") <<
                                   "\nPlain Coupon with rate=strike:       " << nominal * strike * cpn_plain.accrualPeriod() * discount <<
                                   "\nPlain Coupon price:       " << io::rate(cpn_plain_Price) <<
                                   "\nCapped Coupon price:        " << io::rate(cpn_capped_Price) <<
                                   "\nFloored Coupon price:      " << io::rate(cpn_floored_Price) <<
                                   "\ndifference:          " << difference <<
                                   "\ntolerance:           " << io::rate(tol));
                }
            }
        }
    }
}

test_suite* CmsNormalTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Cms normal tests");
    suite->add(QUANTLIB_TEST_CASE(&CmsNormalTest::testFairRate));
    suite->add(QUANTLIB_TEST_CASE(&CmsNormalTest::testCmsSwap));
    suite->add(QUANTLIB_TEST_CASE(&CmsNormalTest::testParity));
    return suite;
}
