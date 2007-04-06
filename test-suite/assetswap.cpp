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
#include <ql/termstructures/volatilities/capletconstantvol.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/analysis.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

QL_BEGIN_TEST_LOCALS(AssetSwapTest)

// global data

Date today_, settlement_;
boost::shared_ptr<IborIndex> index_;
Spread spread_;
DayCounter floatingDayCounter_;
Real faceAmount_;
Calendar calendar_;
//Compounding compounding_ ;
//BusinessDayConvention fixedConvention_, floatingConvention_;
Frequency floatingFrequency_ ; //fixedFrequency_;
Natural settlementDays_;
RelinkableHandle<YieldTermStructure> termStructure_;


void setup() {
    settlementDays_ = 3;
    faceAmount_ = 100.0;
    //fixedConvention_ = Unadjusted;
    //floatingConvention_ = ModifiedFollowing;
    //fixedFrequency_ = Annual;
    floatingFrequency_ = Semiannual;
    floatingDayCounter_= Actual360();
    index_ = boost::shared_ptr<IborIndex>(new
        Euribor(Period(floatingFrequency_), termStructure_));
    spread_=0.0;
    calendar_ = index_->calendar();
    today_ = calendar_.adjust(Date::todaysDate());
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
    DayCounter bondDayCount = ActualActual(ActualActual::ISDA);
    Natural settlementDays = 3;

    bool payFixedRate = true;
    bool parAssetSwap = true;

    // Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    Schedule fixedBondSchedule(Date(4,January,2005),
                               Date(4,January,2037),
                               Period(Annual), bondCalendar,
                               Unadjusted, Unadjusted, true, false);
    boost::shared_ptr<Bond> fixedBond(new
        FixedRateBond(settlementDays, faceAmount_, fixedBondSchedule,
                      std::vector<Rate>(1, 0.04), bondDayCount, Following,
                      100.0, Date(4,January,2005), termStructure_));
    Real fixedBondPrice = fixedBond->cleanPrice();
    AssetSwap fixedBondAssetSwap(payFixedRate, 
                                 fixedBond, fixedBondPrice,
                                 index_, spread_, termStructure_,
                                 Schedule(),
                                 floatingDayCounter_, parAssetSwap);
    Real fixedBondAssetSwapPrice = fixedBondAssetSwap.fairPrice();

    Real tolerance = 1.0e-13;
    Real error = std::fabs(fixedBondAssetSwapPrice-fixedBondPrice);

    if (error>tolerance) {
        BOOST_ERROR("wrong zero spread asset swap price for fixed bond:"
                    << QL_FIXED << std::setprecision(4)
                    << "\n  bond clean price:      " << fixedBondPrice
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice
                    << QL_SCIENTIFIC << std::setprecision(2)
                    << "\n  error:                 " << error
                    << "\n  tolerance:             " << tolerance);
    }

    QL_TEST_TEARDOWN
}

//void AssetSwapTest::testMarketASWSpread() {
//
//    BOOST_MESSAGE("Testing marketassetswap spread= 100*parassetswapspread/bondfullprice...");
//
//    QL_TEST_BEGIN
//    QL_TEST_SETUP
//
//    Calendar bondCalendar = NullCalendar();
//    DayCounter bondDayCount = ActualActual(ActualActual::ISMA);
//    Natural settlementDays = 3;
//
//// Underlying bond
//
//    Schedule bondsch2(Date(31,October,2004),
//                      Date(31,October,2024), Period(Annual), bondCalendar,
//                      Unadjusted, Unadjusted, true,false);
//
//    boost::shared_ptr<Bond> bond2(new FixedRateBond(settlementDays, faceAmount_, bondsch,
//                                  std::vector<Rate>(1, 0.05),bondDayCount, ModifiedFollowing,
//                                  100.0, Date(1,November,2004), termStructure_));
//
//
// //AssetSwap
//
//   Schedule floatSch2(bond2->settlementDate(),
//                     bond2->maturityDate(),index_->tenor(),index_->calendar(),
//                     index_->businessDayConvention(),
//                     Unadjusted,true, index_->endOfMonth());
//
//   upfrontDate_ = floatSch2.startDate();
//
//
//   AssetSwap assetswap2(true, 
//                        bond1,calculatedPrice1,index_,spread_,termStructure_,floatSch,
//                        floatingDayCounter_,true);
//   Real delta1= assetswap1.fairPrice()-calculatedPrice1;
//
//// AssetSwap with spread =0 should return a fair price equal to the bond's theoretical clean price
//
//            if (std::fabs(assetswap1.fairPrice()-calculatedPrice1) > 1.0e-4) {
//                BOOST_ERROR("recalculating with zero spread:\n"
//                            << std::setprecision(7)
//                            << "    difference: " << delta1);
//            }
//
//
//    QL_TEST_TEARDOWN
//}



test_suite* AssetSwapTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("AssetSwap tests");
    suite->add(BOOST_TEST_CASE(&AssetSwapTest::testImpliedValue));
    //suite->add(BOOST_TEST_CASE(&AssetSwapTest::testMarketASWSpread));
    
    return suite;
}

