/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chiara Fornarola

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "assetswap.hpp"
#include "utilities.hpp"
#include <ql/time/schedule.hpp>
#include <ql/instruments/assetswap.hpp>
#include <ql/instruments/fixedratebond.hpp>
#include <ql/instruments/floatingratebond.hpp>
#include <ql/instruments/cmsratebond.hpp>
#include <ql/index.hpp>
#include <ql/termstructures/yieldcurves/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/indexes/euribor.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/termstructures/volatilities/capletconstantvol.hpp>
#include <ql/termstructures/volatilities/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatilities/swaptionvolcube2.hpp>
#include <ql/termstructures/volatilities/swaptionvolcube1.hpp>
#include <ql/termstructures/volatilities/swaptionvolcube.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/analysis.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(AssetSwapTest)

// global data

Date today_, settlement_;
boost::shared_ptr<IborIndex> iborindex_;
boost::shared_ptr<SwapIndex> swapindex_;
Spread spread_;
DayCounter floatingDayCounter_;
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
    floatingDayCounter_= Actual360();
    iborindex_ = boost::shared_ptr<IborIndex>(new
        Euribor(Period(floatingFrequency_), termStructure_));
    calendar_ = iborindex_->calendar();
    swapindex_= boost::shared_ptr<SwapIndex>(new
           SwapIndex("EuriborSwapFixA", 10*Years,swapSettlementDays_,
                iborindex_->currency(), calendar_,
                Period(fixedFrequency_), fixedConvention_,
                iborindex_->dayCounter(), iborindex_));
    spread_=0.0;  
    //today_ = calendar_.adjust(Date::todaysDate());
    Date today_(20,September,2004);
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
    
 /*   Date today_(20,September,2004);
    Settings::instance().evaluationDate() = today_;*/
    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
   
    bool payFixedRate = true;
    bool parAssetSwap = true;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    Schedule fixedBondSchedule(Date(4,January,2005),
                               Date(4,January,2037),
                               Period(Annual), bondCalendar,
                               Unadjusted, Unadjusted, true, false);
    boost::shared_ptr<Bond> fixedBond(new
        FixedRateBond(settlementDays, faceAmount_, fixedBondSchedule,
                      std::vector<Rate>(1, 0.04), ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005), termStructure_));
    Real fixedBondPrice = fixedBond->cleanPrice();
    AssetSwap fixedBondAssetSwap(payFixedRate, 
                                 fixedBond, fixedBondPrice,
                                 iborindex_, spread_, termStructure_,
                                 Schedule(),
                                 floatingDayCounter_, parAssetSwap);
    Real fixedBondAssetSwapPrice = fixedBondAssetSwap.fairPrice();
    Real tolerance = 1.0e-13;
    Real error1 = std::fabs(fixedBondAssetSwapPrice-fixedBondPrice);

    if (error1>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for fixed bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << fixedBondPrice
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance);
    }


    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)

    Schedule floatingBondSchedule(Date(24,September,2004),
                                  Date(24,September,2018),
                                  Period(Semiannual), bondCalendar,
                                  ModifiedFollowing, ModifiedFollowing, 
                                  true, false);
    boost::shared_ptr<IborCouponPricer> pricer(new
    BlackIborCouponPricer(Handle<CapletVolatilityStructure>()));
    bool inArrears = false;
    boost::shared_ptr<Bond> floatingBond(new
        FloatingRateBond(settlementDays, faceAmount_, floatingBondSchedule,
                         iborindex_, Actual360(), 
                         ModifiedFollowing,fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004), termStructure_));
    CashFlows::setPricer(floatingBond->cashflows(),pricer);
    //iborindex_->addFixing(Date(22,March,2007), 0.04013);
    Real floatingBondPrice = floatingBond->cleanPrice();
    AssetSwap floatingBondAssetSwap(payFixedRate, 
                                 floatingBond, floatingBondPrice,
                                 iborindex_, spread_, termStructure_,
                                 Schedule(),
                                 floatingDayCounter_, parAssetSwap);
    Real floatingBondAssetSwapPrice = floatingBondAssetSwap.fairPrice();
    Real error2 = std::fabs(floatingBondAssetSwapPrice-floatingBondPrice);

    if (error2>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for floating bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << floatingBondPrice
                    << "\n  asset swap fair price: " << floatingBondAssetSwapPrice
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error2
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

