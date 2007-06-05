/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola

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

#include "assetswap.hpp"
#include "utilities.hpp"
#include <ql/time/schedule.hpp>
#include <ql/instruments/assetswap.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/bonds/cmsratebond.hpp>
#include <ql/index.hpp>
#include <ql/termstructures/yieldcurves/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/termstructures/volatilities/caplet/capletconstantvol.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatilities/swaption/swaptionvolcube.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(AssetSwapTest)

// global data

Date today_, settlement_;
boost::shared_ptr<IborIndex> iborIndex_;
boost::shared_ptr<SwapIndex> swapIndex_;
Spread spread_;
Real faceAmount_;
Calendar calendar_;
//Compounding compounding_ ;
BusinessDayConvention fixedConvention_; //, floatingConvention_;
Frequency floatingFrequency_ , fixedFrequency_;
Natural settlementDays_ , swapSettlementDays_;
RelinkableHandle<YieldTermStructure> termStructure_;


void setup() {
    settlementDays_ = 3;
    swapSettlementDays_ = 2;
    faceAmount_ = 100.0;
    fixedConvention_ = Unadjusted;
    //floatingConvention_ = ModifiedFollowing;
    fixedFrequency_ = Annual;
    floatingFrequency_ = Semiannual;
    iborIndex_ = boost::shared_ptr<IborIndex>(new
        Euribor(Period(floatingFrequency_), termStructure_));
    calendar_ = iborIndex_->fixingCalendar();
    swapIndex_= boost::shared_ptr<SwapIndex>(new
           SwapIndex("EuriborSwapFixA", 10*Years,swapSettlementDays_,
                iborIndex_->currency(), calendar_,
                Period(fixedFrequency_), fixedConvention_,
                iborIndex_->dayCounter(), iborIndex_));
    spread_=0.0;
    //today_ = calendar_.adjust(Date::todaysDate());
    Date today_(24,April,2007);
    Settings::instance().evaluationDate() = today_;
    //settlement_ = calendar_.advance(today_,settlementDays_,Days);
    termStructure_.linkTo(flatRate(today_,0.05,Actual365Fixed()));
}

void teardown() {
    Settings::instance().evaluationDate() = Date();
}

QL_END_TEST_LOCALS(AssetSwapTest)


void AssetSwapTest::testImpliedValue() {

    BOOST_MESSAGE("Testing asset swap bond price...");

    QL_TEST_BEGIN
    QL_TEST_SETUP

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;

    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool inArrears = false;
    boost::shared_ptr<IborCouponPricer> pricer(new
    BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
    Handle<SwaptionVolatilityStructure> swaptionVolatilityStructure(
           boost::shared_ptr<SwaptionVolatilityStructure>(new
           SwaptionConstantVolatility(Settings::instance().evaluationDate(),0.2,
                                      Actual365Fixed())));
    Handle<Quote> meanReversionQuote(boost::shared_ptr<Quote>(new SimpleQuote(0.01)));
    GFunctionFactory::ModelOfYieldCurve modelOfYieldCurve_;
    modelOfYieldCurve_= GFunctionFactory::Standard;
    boost::shared_ptr<CmsCouponPricer> cmspricer(new
                      ConundrumPricerByBlack(swaptionVolatilityStructure,
                      modelOfYieldCurve_, meanReversionQuote));

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day
    Schedule fixedBondSchedule1(Date(4,January,2005),
                                Date(4,January,2037),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted, true, false);
    boost::shared_ptr<Bond> fixedBond1(new
        FixedRateBond(settlementDays, faceAmount_, fixedBondSchedule1,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005), termStructure_));
    Real fixedBondPrice1 = fixedBond1->cleanPrice();
    AssetSwap fixedBondAssetSwap1(payFixedRate,
                                  fixedBond1, fixedBondPrice1,
                                  iborIndex_, spread_, termStructure_,
                                  Schedule(),
                                  iborIndex_->dayCounter(),
                                  parAssetSwap);
    Real fixedBondAssetSwapPrice1 = fixedBondAssetSwap1.fairPrice();
    Real tolerance = 1.0e-13;
    Real error1 = std::fabs(fixedBondAssetSwapPrice1-fixedBondPrice1);

    if (error1>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for fixed bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << fixedBondPrice1
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice1
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance);
    }


    
    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day
    Schedule fixedBondSchedule2(Date(5,February,2005),
                                Date(5,February,2019),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted, true, false);
    boost::shared_ptr<Bond> fixedBond2(new
        FixedRateBond(settlementDays, faceAmount_, fixedBondSchedule2,
                      std::vector<Rate>(1, 0.05),
                      Thirty360(Thirty360::BondBasis), Following,
                      100.0, Date(5,February,2005), termStructure_));
    Real fixedBondPrice2 = fixedBond2->cleanPrice();
    AssetSwap fixedBondAssetSwap2(payFixedRate,
                                  fixedBond2, fixedBondPrice2,
                                  iborIndex_, spread_, termStructure_,
                                  Schedule(),
                                  iborIndex_->dayCounter(),
                                  parAssetSwap);
    Real fixedBondAssetSwapPrice2 = fixedBondAssetSwap2.fairPrice();
    //Real tolerance = 1.0e-13;
    Real error2 = std::fabs(fixedBondAssetSwapPrice2-fixedBondPrice2);

    if (error2>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for fixed bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << fixedBondPrice2
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice2
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error2
                    << "\n  tolerance:             " << tolerance);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Schedule floatingBondSchedule1(Date(29,September,2003),
                                   Date(29,September,2013),
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Following,
                                   true, false);
    
    boost::shared_ptr<Bond> floatingBond1(new
        FloatingRateBond(settlementDays, faceAmount_, floatingBondSchedule1,
                         iborIndex_, Actual360(),
                         Following, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0056),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(29,September,2003), termStructure_));
    setCouponPricer(floatingBond1->cashflows(),pricer);
    iborIndex_->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondPrice1 = floatingBond1->cleanPrice();
    AssetSwap floatingBondAssetSwap1(payFixedRate,
                                     floatingBond1, floatingBondPrice1,
                                     iborIndex_, spread_, termStructure_,
                                     Schedule(),
                                     iborIndex_->dayCounter(),
                                     parAssetSwap);
    Real floatingBondAssetSwapPrice1 = floatingBondAssetSwap1.fairPrice();
    Real error3 = std::fabs(floatingBondAssetSwapPrice1-floatingBondPrice1);

    if (error3>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for floating bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << floatingBondPrice1
                    << "\n  asset swap fair price: " << floatingBondAssetSwapPrice1
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error3
                    << "\n  tolerance:             " << tolerance);
    }



    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Schedule floatingBondSchedule2(Date(24,September,2004),
                                   Date(24,September,2018),
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   true, false);
    boost::shared_ptr<Bond> floatingBond2(new
        FloatingRateBond(settlementDays, faceAmount_, floatingBondSchedule2,
                         iborIndex_, Actual360(),
                         ModifiedFollowing, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004), termStructure_));
    setCouponPricer(floatingBond2->cashflows(),pricer);
    iborIndex_->addFixing(Date(22,March,2007), 0.04013);
    Real currentCoupon=0.04013+0.0025;
    Real floatingCurrentCoupon= floatingBond2->currentCoupon();
    Real error4= std::fabs(floatingCurrentCoupon-currentCoupon);
    if (error4>tolerance) {
        BOOST_ERROR("wrong current coupon is returned for floating bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond calculated current coupon:      " << currentCoupon
                    << "\n  current coupon asked to the bond: " << floatingCurrentCoupon
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error4
                    << "\n  tolerance:             " << tolerance);
    }


    Real floatingBondPrice2 = floatingBond2->cleanPrice();
    AssetSwap floatingBondAssetSwap2(payFixedRate,
                                     floatingBond2, floatingBondPrice2,
                                     iborIndex_, spread_, termStructure_,
                                     Schedule(),
                                     iborIndex_->dayCounter(),
                                     parAssetSwap);
    Real floatingBondAssetSwapPrice2 = floatingBondAssetSwap2.fairPrice();
    Real error5 = std::fabs(floatingBondAssetSwapPrice2-floatingBondPrice2);

    if (error5>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for floating bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << floatingBondPrice2
                    << "\n  asset swap fair price: " << floatingBondAssetSwapPrice2
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error5
                    << "\n  tolerance:             " << tolerance);
    }
    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day

    Schedule cmsBondSchedule1(Date(22,August,2005),
                              Date(22,August,2020),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              true, false);
    boost::shared_ptr<Bond> cmsBond1(new
        CmsRateBond(settlementDays, faceAmount_, cmsBondSchedule1,
                    swapIndex_, Thirty360(),
                    Following, fixingDays,
                    std::vector<Real>(1,1.0), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(1,0.055), std::vector<Rate>(1,0.025),
                    inArrears,
                    100.0, Date(22,August,2005), termStructure_));
    setCouponPricer(cmsBond1->cashflows(),cmspricer);
    swapIndex_->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondPrice1 = cmsBond1->cleanPrice();
    AssetSwap cmsBondAssetSwap1(payFixedRate,
                                cmsBond1, cmsBondPrice1,
                                iborIndex_, spread_, termStructure_,
                                Schedule(),
                                iborIndex_->dayCounter(),
                                parAssetSwap);
    Real cmsBondAssetSwapPrice1 = cmsBondAssetSwap1.fairPrice();
    Real error6 = std::fabs(cmsBondAssetSwapPrice1-cmsBondPrice1);

    if (error6>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for cms bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << cmsBondPrice1
                    << "\n  asset swap fair price: " << cmsBondAssetSwapPrice1
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error6
                    << "\n  tolerance:             " << tolerance);
    }

     // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
     // maturity occurs on a business day

    Schedule cmsBondSchedule2(Date(06,May,2005),
                              Date(06,May,2015),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              true, false);
    boost::shared_ptr<Bond> cmsBond2(new
        CmsRateBond(settlementDays, faceAmount_, cmsBondSchedule2,
                    swapIndex_, Thirty360(),
                    Following, fixingDays,
                    std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(), std::vector<Rate>(),
                    inArrears,
                    100.0, Date(06,May,2005), termStructure_));
    setCouponPricer(cmsBond2->cashflows(),cmspricer);
    swapIndex_->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondPrice2 = cmsBond2->cleanPrice();
    AssetSwap cmsBondAssetSwap2(payFixedRate,
                                cmsBond2, cmsBondPrice2,
                                iborIndex_, spread_, termStructure_,
                                Schedule(),
                                iborIndex_->dayCounter(),
                                parAssetSwap);
    Real cmsBondAssetSwapPrice2 = cmsBondAssetSwap2.fairPrice();
    Real error7 = std::fabs(cmsBondAssetSwapPrice2-cmsBondPrice2);

    if (error7>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for cms bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << cmsBondPrice2
                    << "\n  asset swap fair price: " << cmsBondAssetSwapPrice2
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance);
    }

      QL_TEST_TEARDOWN
}

test_suite* AssetSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("AssetSwap tests");
    suite->add(BOOST_TEST_CASE(&AssetSwapTest::testImpliedValue));
    //suite->add(BOOST_TEST_CASE(&AssetSwapTest::testMarketASWSpread));

    return suite;
}
