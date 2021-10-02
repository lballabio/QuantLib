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
#include <ql/instruments/bond.hpp>
#include <ql/instruments/bonds/fixedratebond.hpp>
#include <ql/instruments/bonds/floatingratebond.hpp>
#include <ql/instruments/bonds/cmsratebond.hpp>
#include <ql/instruments/bonds/zerocouponbond.hpp>
#include <ql/index.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/thirty360.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/simpledaycounter.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cmscoupon.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/cashflows/conundrumpricer.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/termstructures/volatility/swaption/swaptionconstantvol.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube2.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/pricingengines/bond/discountingbondengine.hpp>
#include <ql/pricingengines/bond/bondfunctions.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace asset_swap_test {

    struct CommonVars {
        // common data
        ext::shared_ptr<IborIndex> iborIndex;
        ext::shared_ptr<SwapIndex> swapIndex;
        ext::shared_ptr<IborCouponPricer> pricer;
        ext::shared_ptr<CmsCouponPricer> cmspricer;
        Spread spread;
        Spread nonnullspread;
        Real faceAmount;
        Compounding compounding;
        RelinkableHandle<YieldTermStructure> termStructure;

        // clean-up
        SavedSettings backup;
        IndexHistoryCleaner indexCleaner;

        // initial setup
        CommonVars() {
            Natural swapSettlementDays = 2;
            faceAmount = 100.0;
            BusinessDayConvention fixedConvention = Unadjusted;
            compounding = Continuous;
            Frequency fixedFrequency = Annual;
            Frequency floatingFrequency = Semiannual;
            iborIndex = ext::shared_ptr<IborIndex>(
                     new Euribor(Period(floatingFrequency), termStructure));
            Calendar calendar = iborIndex->fixingCalendar();
            swapIndex= ext::make_shared<SwapIndex>(
                "EuriborSwapIsdaFixA", 10*Years, swapSettlementDays,
                              iborIndex->currency(), calendar,
                              Period(fixedFrequency), fixedConvention,
                              iborIndex->dayCounter(), iborIndex);
            spread = 0.0;
            nonnullspread = 0.003;
            Date today(24,April,2007);
            Settings::instance().evaluationDate() = today;

            //Date today = Settings::instance().evaluationDate();

            termStructure.linkTo(flatRate(today, 0.05, Actual365Fixed()));
            pricer = ext::shared_ptr<IborCouponPricer>(new
                                                        BlackIborCouponPricer);
            Handle<SwaptionVolatilityStructure> swaptionVolatilityStructure(
                ext::shared_ptr<SwaptionVolatilityStructure>(new
                    ConstantSwaptionVolatility(today, NullCalendar(),Following,
                                               0.2, Actual365Fixed())));
            Handle<Quote> meanReversionQuote(ext::shared_ptr<Quote>(new
                SimpleQuote(0.01)));
            cmspricer = ext::shared_ptr<CmsCouponPricer>(new
                AnalyticHaganPricer(swaptionVolatilityStructure,
                                    GFunctionFactory::Standard,
                                    meanReversionQuote));
        }
    };

}

void AssetSwapTest::testConsistency() {
    BOOST_TEST_MESSAGE(
                 "Testing consistency between fair price and fair spread...");

    using namespace asset_swap_test;

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Schedule bondSchedule(Date(4,January,2005),
                          Date(4,January,2037),
                          Period(Annual), bondCalendar,
                          Unadjusted, Unadjusted,
                          DateGeneration::Backward, false);
    ext::shared_ptr<Bond> bond(new
        FixedRateBond(settlementDays, vars.faceAmount,
                      bondSchedule,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA),
                      Following,
                      100.0, Date(4,January,2005)));

    bool payFixedRate = true;
    Real bondPrice = 95.0;

    bool isPar = true;
    AssetSwap parAssetSwap(payFixedRate,
                         bond, bondPrice,
                         vars.iborIndex, vars.spread,
                         Schedule(),
                         vars.iborIndex->dayCounter(),
                         isPar);

    ext::shared_ptr<PricingEngine> swapEngine(new
        DiscountingSwapEngine(vars.termStructure,
                              true,
                              bond->settlementDate(),
                              Settings::instance().evaluationDate()));

    parAssetSwap.setPricingEngine(swapEngine);
    Real fairCleanPrice = parAssetSwap.fairCleanPrice();
    Spread fairSpread = parAssetSwap.fairSpread();

    Real tolerance = 1.0e-13;

    AssetSwap assetSwap2(payFixedRate,
                         bond, fairCleanPrice,
                         vars.iborIndex, vars.spread,
                         Schedule(),
                         vars.iborIndex->dayCounter(),
                         isPar);
    assetSwap2.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap2.NPV())>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  clean price:      " << bondPrice <<
                   "\n  fair clean price: " << fairCleanPrice <<
                   "\n  NPV:              " << assetSwap2.NPV() <<
                   "\n  tolerance:        " << tolerance);
    }
    if (std::fabs(assetSwap2.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << fairCleanPrice <<
                   "\n  fair clean price:  " << assetSwap2.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap2.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap2.fairSpread() - vars.spread)>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't equal input spread "
                   "at zero NPV: " << std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << vars.spread <<
                   "\n  fair spread:  " << assetSwap2.fairSpread() <<
                   "\n  NPV:          " << assetSwap2.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

    AssetSwap assetSwap3(payFixedRate,
                         bond, bondPrice,
                         vars.iborIndex, fairSpread,
                         Schedule(),
                         vars.iborIndex->dayCounter(),
                         isPar);
    assetSwap3.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap3.NPV())>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  spread:      " << vars.spread <<
                   "\n  fair spread: " << fairSpread <<
                   "\n  NPV:         " << assetSwap3.NPV() <<
                   "\n  tolerance:   " << tolerance);
    }
    if (std::fabs(assetSwap3.fairCleanPrice() - bondPrice)>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << bondPrice <<
                   "\n  fair clean price:  " << assetSwap3.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap3.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap3.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't equal input spread at"
                   " zero NPV: " << std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << fairSpread <<
                   "\n  fair spread:  " << assetSwap3.fairSpread() <<
                   "\n  NPV:          " << assetSwap3.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

    // let's change the npv date
    swapEngine = ext::shared_ptr<PricingEngine>(new
        DiscountingSwapEngine(vars.termStructure,
                              true,
                              bond->settlementDate(),
                              bond->settlementDate()));

    parAssetSwap.setPricingEngine(swapEngine);
    // fair clean price and fair spread should not change
    if (std::fabs(parAssetSwap.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price changed with NpvDate:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n expected clean price: " << fairCleanPrice <<
                   "\n fair clean price:     "<<parAssetSwap.fairCleanPrice()<<
                   "\n tolerance:            " << tolerance);
    }
    if (std::fabs(parAssetSwap.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread changed with NpvDate:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  expected spread: " << fairSpread <<
                   "\n  fair spread:     " << parAssetSwap.fairSpread() <<
                   "\n  tolerance:       " << tolerance);
    }

    assetSwap2 = AssetSwap(payFixedRate,
                           bond, fairCleanPrice,
                           vars.iborIndex, vars.spread,
                           Schedule(),
                           vars.iborIndex->dayCounter(),
                           isPar);
    assetSwap2.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap2.NPV())>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  clean price:      " << bondPrice <<
                   "\n  fair clean price: " << fairCleanPrice <<
                   "\n  NPV:              " << assetSwap2.NPV() <<
                   "\n  tolerance:        " << tolerance);
    }
    if (std::fabs(assetSwap2.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << fairCleanPrice <<
                   "\n  fair clean price:  " << assetSwap2.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap2.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap2.fairSpread() - vars.spread)>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't equal input spread at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << vars.spread <<
                   "\n  fair spread:  " << assetSwap2.fairSpread() <<
                   "\n  NPV:          " << assetSwap2.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

    assetSwap3 = AssetSwap(payFixedRate,
                           bond, bondPrice,
                           vars.iborIndex, fairSpread,
                           Schedule(),
                           vars.iborIndex->dayCounter(),
                           isPar);
    assetSwap3.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap3.NPV())>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  spread:      " << vars.spread <<
                   "\n  fair spread: " << fairSpread <<
                   "\n  NPV:         " << assetSwap3.NPV() <<
                   "\n  tolerance:   " << tolerance);
    }
    if (std::fabs(assetSwap3.fairCleanPrice() - bondPrice)>tolerance) {
        BOOST_FAIL("\npar asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << bondPrice <<
                   "\n  fair clean price:  " << assetSwap3.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap3.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap3.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\npar asset swap fair spread doesn't equal input spread at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << fairSpread <<
                   "\n  fair spread:  " << assetSwap3.fairSpread() <<
                   "\n  NPV:          " << assetSwap3.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }





    // now market asset swap
    isPar = false;
    AssetSwap mktAssetSwap(payFixedRate,
                           bond, bondPrice,
                           vars.iborIndex, vars.spread,
                           Schedule(),
                           vars.iborIndex->dayCounter(),
                           isPar);

    swapEngine = ext::shared_ptr<PricingEngine>(new
        DiscountingSwapEngine(vars.termStructure,
                              true,
                              bond->settlementDate(),
                              Settings::instance().evaluationDate()));

    mktAssetSwap.setPricingEngine(swapEngine);
    fairCleanPrice = mktAssetSwap.fairCleanPrice();
    fairSpread = mktAssetSwap.fairSpread();

    AssetSwap assetSwap4(payFixedRate,
                         bond, fairCleanPrice,
                         vars.iborIndex, vars.spread,
                         Schedule(),
                         vars.iborIndex->dayCounter(),
                         isPar);
    assetSwap4.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap4.NPV())>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  clean price:      " << bondPrice <<
                   "\n  fair clean price: " << fairCleanPrice <<
                   "\n  NPV:              " << assetSwap4.NPV() <<
                   "\n  tolerance:        " << tolerance);
    }
    if (std::fabs(assetSwap4.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << fairCleanPrice <<
                   "\n  fair clean price:  " << assetSwap4.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap4.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap4.fairSpread() - vars.spread)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't equal input spread"
                   " at zero NPV: " << std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << vars.spread <<
                   "\n  fair spread:  " << assetSwap4.fairSpread() <<
                   "\n  NPV:          " << assetSwap4.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

    AssetSwap assetSwap5(payFixedRate,
                         bond, bondPrice,
                         vars.iborIndex, fairSpread,
                         Schedule(),
                         vars.iborIndex->dayCounter(),
                         isPar);
    assetSwap5.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap5.NPV())>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  spread:      " << vars.spread <<
                   "\n  fair spread: " << fairSpread <<
                   "\n  NPV:         " << assetSwap5.NPV() <<
                   "\n  tolerance:   " << tolerance);
    }
    if (std::fabs(assetSwap5.fairCleanPrice() - bondPrice)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << bondPrice <<
                   "\n  fair clean price:  " << assetSwap5.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap5.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap5.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't equal input spread at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << fairSpread <<
                   "\n  fair spread:  " << assetSwap5.fairSpread() <<
                   "\n  NPV:          " << assetSwap5.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

    // let's change the npv date
    swapEngine = ext::shared_ptr<PricingEngine>(new
        DiscountingSwapEngine(vars.termStructure,
                              true,
                              bond->settlementDate(),
                              bond->settlementDate()));

    mktAssetSwap.setPricingEngine(swapEngine);
    // fair clean price and fair spread should not change
    if (std::fabs(mktAssetSwap.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price changed with NpvDate:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  expected clean price: " << fairCleanPrice <<
                   "\n  fair clean price:  " << mktAssetSwap.fairCleanPrice() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(mktAssetSwap.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread changed with NpvDate:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  expected spread: " << fairSpread <<
                   "\n  fair spread:  " << mktAssetSwap.fairSpread() <<
                   "\n  tolerance:    " << tolerance);
    }

    assetSwap4 = AssetSwap(payFixedRate,
                           bond, fairCleanPrice,
                           vars.iborIndex, vars.spread,
                           Schedule(),
                           vars.iborIndex->dayCounter(),
                           isPar);
    assetSwap4.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap4.NPV())>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  clean price:      " << bondPrice <<
                   "\n  fair clean price: " << fairCleanPrice <<
                   "\n  NPV:              " << assetSwap4.NPV() <<
                   "\n  tolerance:        " << tolerance);
    }
    if (std::fabs(assetSwap4.fairCleanPrice() - fairCleanPrice)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << fairCleanPrice <<
                   "\n  fair clean price:  " << assetSwap4.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap4.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap4.fairSpread() - vars.spread)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't equal input spread at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << vars.spread <<
                   "\n  fair spread:  " << assetSwap4.fairSpread() <<
                   "\n  NPV:          " << assetSwap4.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

     assetSwap5 = AssetSwap(payFixedRate,
                            bond, bondPrice,
                            vars.iborIndex, fairSpread,
                            Schedule(),
                            vars.iborIndex->dayCounter(),
                            isPar);
    assetSwap5.setPricingEngine(swapEngine);
    if (std::fabs(assetSwap5.NPV())>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't zero the NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  spread:      " << vars.spread <<
                   "\n  fair spread: " << fairSpread <<
                   "\n  NPV:         " << assetSwap5.NPV() <<
                   "\n  tolerance:   " << tolerance);
    }
    if (std::fabs(assetSwap5.fairCleanPrice() - bondPrice)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair clean price doesn't equal input "
                   "clean price at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input clean price: " << bondPrice <<
                   "\n  fair clean price:  " << assetSwap5.fairCleanPrice() <<
                   "\n  NPV:               " << assetSwap5.NPV() <<
                   "\n  tolerance:         " << tolerance);
    }
    if (std::fabs(assetSwap5.fairSpread() - fairSpread)>tolerance) {
        BOOST_FAIL("\nmarket asset swap fair spread doesn't equal input spread at zero NPV: " <<
                   std::fixed << std::setprecision(4) <<
                   "\n  input spread: " << fairSpread <<
                   "\n  fair spread:  " << assetSwap5.fairSpread() <<
                   "\n  NPV:          " << assetSwap5.NPV() <<
                   "\n  tolerance:    " << tolerance);
    }

}

void AssetSwapTest::testImpliedValue() {

    BOOST_TEST_MESSAGE("Testing implied bond value against asset-swap fair"
                       " price with null spread...");

    using namespace asset_swap_test;

    const auto & iborcoupon_settings = IborCoupon::Settings::instance();

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Schedule fixedBondSchedule1(Date(4,January,2005),
                                Date(4,January,2037),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond1(
                         new FixedRateBond(settlementDays, vars.faceAmount,
                                           fixedBondSchedule1,
                                           std::vector<Rate>(1, 0.04),
                                           ActualActual(ActualActual::ISDA),
                                           Following,
                                           100.0, Date(4,January,2005)));

    ext::shared_ptr<PricingEngine> bondEngine(
                            new DiscountingBondEngine(vars.termStructure));
    ext::shared_ptr<PricingEngine> swapEngine(
                            new DiscountingSwapEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondPrice1 = fixedBond1->cleanPrice();
    AssetSwap fixedBondAssetSwap1(payFixedRate,
                                  fixedBond1, fixedBondPrice1,
                                  vars.iborIndex, vars.spread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice1 = fixedBondAssetSwap1.fairCleanPrice();
    Real tolerance = 1.0e-13;

    // for indexed coupons the float leg will not be par, therefore we
    // have to relax the tolerance - note that the fair clean price is
    // correct though, only we can not compare it to the bond price
    // directly. The same kind of discrepancy will occur for a multi
    // curve set up, which we do not test here.
    Real tolerance2;
    if (!iborcoupon_settings.usingAtParCoupons())
        tolerance2 = 1.0e-2;
    else
        tolerance2 = 1.0e-13;

    Real error1 = std::fabs(fixedBondAssetSwapPrice1-fixedBondPrice1);

    if (error1>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for fixed bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  bond's clean price:    " << fixedBondPrice1 <<
                    "\n  asset swap fair price: " << fixedBondAssetSwapPrice1 <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                 " << error1 <<
                    "\n  tolerance:             " << tolerance2);
    }

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day

    Schedule fixedBondSchedule2(Date(5,February,2005),
                                Date(5,February,2019),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond2(
                         new FixedRateBond(settlementDays, vars.faceAmount,
                                           fixedBondSchedule2,
                                           std::vector<Rate>(1, 0.05),
                                           Thirty360(Thirty360::BondBasis),
                                           Following,
                                           100.0, Date(5,February,2005)));

    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondPrice2 = fixedBond2->cleanPrice();
    AssetSwap fixedBondAssetSwap2(payFixedRate,
                                  fixedBond2, fixedBondPrice2,
                                  vars.iborIndex, vars.spread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice2 = fixedBondAssetSwap2.fairCleanPrice();
    Real error2 = std::fabs(fixedBondAssetSwapPrice2-fixedBondPrice2);

    if (error2>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << fixedBondPrice2 <<
                   "\n  asset swap fair price: " << fixedBondAssetSwapPrice2 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error2 <<
                   "\n  tolerance:             " << tolerance2);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Schedule floatingBondSchedule1(Date(29,September,2003),
                                   Date(29,September,2013),
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);

    ext::shared_ptr<Bond> floatingBond1(
                      new FloatingRateBond(settlementDays, vars.faceAmount,
                                           floatingBondSchedule1,
                                           vars.iborIndex, Actual360(),
                                           Following, fixingDays,
                                           std::vector<Real>(1,1),
                                           std::vector<Spread>(1,0.0056),
                                           std::vector<Rate>(),
                                           std::vector<Rate>(),
                                           inArrears,
                                           100.0, Date(29,September,2003)));

    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondPrice1 = floatingBond1->cleanPrice();
    AssetSwap floatingBondAssetSwap1(payFixedRate,
                                     floatingBond1, floatingBondPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice1 = floatingBondAssetSwap1.fairCleanPrice();
    Real error3 = std::fabs(floatingBondAssetSwapPrice1-floatingBondPrice1);

    if (error3>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for floater:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << floatingBondPrice1 <<
                   "\n  asset swap fair price: " << floatingBondAssetSwapPrice1 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error3 <<
                   "\n  tolerance:             " << tolerance2);
    }

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Schedule floatingBondSchedule2(Date(24,September,2004),
                                   Date(24,September,2018),
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    ext::shared_ptr<Bond> floatingBond2(
                      new FloatingRateBond(settlementDays, vars.faceAmount,
                                           floatingBondSchedule2,
                                           vars.iborIndex, Actual360(),
                                           ModifiedFollowing, fixingDays,
                                           std::vector<Real>(1,1),
                                           std::vector<Spread>(1,0.0025),
                                           std::vector<Rate>(),
                                           std::vector<Rate>(),
                                           inArrears,
                                           100.0, Date(24,September,2004)));

    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    Real currentCoupon=0.04013+0.0025;
    Rate floatingCurrentCoupon= floatingBond2->nextCouponRate();
    Real error4= std::fabs(floatingCurrentCoupon-currentCoupon);
    if (error4>tolerance) {
        BOOST_FAIL("wrong current coupon is returned for floater bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's calculated current coupon:      " <<
                   currentCoupon <<
                   "\n  current coupon asked to the bond: " <<
                   floatingCurrentCoupon <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error4 <<
                   "\n  tolerance:             " << tolerance);
    }

    Real floatingBondPrice2 = floatingBond2->cleanPrice();
    AssetSwap floatingBondAssetSwap2(payFixedRate,
                                     floatingBond2, floatingBondPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap2.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice2 = floatingBondAssetSwap2.fairCleanPrice();
    Real error5 = std::fabs(floatingBondAssetSwapPrice2-floatingBondPrice2);

    if (error5>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for floater:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << floatingBondPrice2 <<
                   "\n  asset swap fair price: " << floatingBondAssetSwapPrice2 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error5 <<
                   "\n  tolerance:             " << tolerance2);
    }

    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day

    Schedule cmsBondSchedule1(Date(22,August,2005),
                              Date(22,August,2020),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond1(
                          new CmsRateBond(settlementDays, vars.faceAmount,
                                          cmsBondSchedule1,
                                          vars.swapIndex, Thirty360(Thirty360::BondBasis),
                                          Following, fixingDays,
                                          std::vector<Real>(1,1.0),
                                          std::vector<Spread>(1,0.0),
                                          std::vector<Rate>(1,0.055),
                                          std::vector<Rate>(1,0.025),
                                          inArrears,
                                          100.0, Date(22,August,2005)));

    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondPrice1 = cmsBond1->cleanPrice();
    AssetSwap cmsBondAssetSwap1(payFixedRate,
                                cmsBond1, cmsBondPrice1,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice1 = cmsBondAssetSwap1.fairCleanPrice();
    Real error6 = std::fabs(cmsBondAssetSwapPrice1-cmsBondPrice1);

    if (error6>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for cms bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << cmsBondPrice1 <<
                   "\n  asset swap fair price: " << cmsBondAssetSwapPrice1 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error6 <<
                   "\n  tolerance:             " << tolerance2);
    }

     // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
     // maturity occurs on a business day

    Schedule cmsBondSchedule2(Date(06,May,2005),
                              Date(06,May,2015),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond2(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule2,
                    vars.swapIndex, Thirty360(Thirty360::BondBasis),
                    Following, fixingDays,
                    std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(), std::vector<Rate>(),
                    inArrears,
                    100.0, Date(06,May,2005)));

    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondPrice2 = cmsBond2->cleanPrice();
    AssetSwap cmsBondAssetSwap2(payFixedRate,
                                cmsBond2, cmsBondPrice2,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap2.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice2 = cmsBondAssetSwap2.fairCleanPrice();
    Real error7 = std::fabs(cmsBondAssetSwapPrice2-cmsBondPrice2);

    if (error7>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for cms bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << cmsBondPrice2 <<
                   "\n  asset swap fair price: " << cmsBondAssetSwapPrice2 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error7 <<
                   "\n  tolerance:             " << tolerance2);
    }

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day

    ext::shared_ptr<Bond> zeroCpnBond1(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(20,December,2015),
                       Following,
                       100.0, Date(19,December,1985)));

    zeroCpnBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice1 = zeroCpnBond1->cleanPrice();
    AssetSwap zeroCpnAssetSwap1(payFixedRate,
                                zeroCpnBond1, zeroCpnBondPrice1,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    zeroCpnAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice1 = zeroCpnAssetSwap1.fairCleanPrice();
    Real error8 = std::fabs(cmsBondAssetSwapPrice1-cmsBondPrice1);

    if (error8>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for zero cpn bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:    " << zeroCpnBondPrice1 <<
                   "\n  asset swap fair price: " << zeroCpnBondAssetSwapPrice1 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error8 <<
                   "\n  tolerance:             " << tolerance2);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day

    ext::shared_ptr<Bond> zeroCpnBond2(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(17,February,2028),
                       Following,
                       100.0, Date(17,February,1998)));

    zeroCpnBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice2 = zeroCpnBond2->cleanPrice();
    AssetSwap zeroCpnAssetSwap2(payFixedRate,
                                zeroCpnBond2, zeroCpnBondPrice2,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    zeroCpnAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice2 = zeroCpnAssetSwap2.fairCleanPrice();
    Real error9 = std::fabs(cmsBondAssetSwapPrice2-cmsBondPrice2);

    if (error9>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for zero cpn bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  bond's clean price:      " << zeroCpnBondPrice2 <<
                   "\n  asset swap fair price:   " << zeroCpnBondAssetSwapPrice2 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                   " << error9 <<
                   "\n  tolerance:               " << tolerance2);
    }
}


void AssetSwapTest::testMarketASWSpread() {

    BOOST_TEST_MESSAGE("Testing relationship between market asset swap"
                       " and par asset swap...");

    using namespace asset_swap_test;

    const auto & iborcoupon_settings = IborCoupon::Settings::instance();

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool mktAssetSwap = false;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Schedule fixedBondSchedule1(Date(4,January,2005),
                                Date(4,January,2037),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond1(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule1,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005)));

    ext::shared_ptr<PricingEngine> bondEngine(
                            new DiscountingBondEngine(vars.termStructure));
    ext::shared_ptr<PricingEngine> swapEngine(
                            new DiscountingSwapEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondMktPrice1 = 89.22 ; // market price observed on 7th June 2007
    Real fixedBondMktFullPrice1=fixedBondMktPrice1+fixedBond1->accruedAmount();
    AssetSwap fixedBondParAssetSwap1(payFixedRate,
                                     fixedBond1, fixedBondMktPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    fixedBondParAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondParAssetSwapSpread1 = fixedBondParAssetSwap1.fairSpread();
    AssetSwap fixedBondMktAssetSwap1(payFixedRate,
                                     fixedBond1, fixedBondMktPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     mktAssetSwap);
    fixedBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondMktAssetSwapSpread1 = fixedBondMktAssetSwap1.fairSpread();

    // see comment above
    Real tolerance2;
    if (!iborcoupon_settings.usingAtParCoupons())
        tolerance2 = 1.0e-4;
    else
        tolerance2 = 1.0e-13;

    Real error1 =
        std::fabs(fixedBondMktAssetSwapSpread1-
                  100*fixedBondParAssetSwapSpread1/fixedBondMktFullPrice1);

    if (error1>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(fixedBondMktAssetSwapSpread1) <<
                   "\n  par ASW spread:    " << io::rate(fixedBondParAssetSwapSpread1) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error1 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day

    Schedule fixedBondSchedule2(Date(5,February,2005),
                                Date(5,February,2019),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond2(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule2,
                      std::vector<Rate>(1, 0.05),
                      Thirty360(Thirty360::BondBasis), Following,
                      100.0, Date(5,February,2005)));

    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondMktPrice2 = 99.98 ; // market price observed on 7th June 2007
    Real fixedBondMktFullPrice2=fixedBondMktPrice2+fixedBond2->accruedAmount();
    AssetSwap fixedBondParAssetSwap2(payFixedRate,
                                     fixedBond2, fixedBondMktPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    fixedBondParAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondParAssetSwapSpread2 = fixedBondParAssetSwap2.fairSpread();
    AssetSwap fixedBondMktAssetSwap2(payFixedRate,
                                     fixedBond2, fixedBondMktPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     mktAssetSwap);
    fixedBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondMktAssetSwapSpread2 = fixedBondMktAssetSwap2.fairSpread();
    Real error2 =
        std::fabs(fixedBondMktAssetSwapSpread2-
                  100*fixedBondParAssetSwapSpread2/fixedBondMktFullPrice2);

    if (error2>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(fixedBondMktAssetSwapSpread2) <<
                   "\n  par ASW spread:    " << io::rate(fixedBondParAssetSwapSpread2) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error2 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Schedule floatingBondSchedule1(Date(29,September,2003),
                                   Date(29,September,2013),
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);

    ext::shared_ptr<Bond> floatingBond1(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule1,
                         vars.iborIndex, Actual360(),
                         Following, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0056),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(29,September,2003)));

    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    // market price observed on 7th June 2007
    Real floatingBondMktPrice1 = 101.64 ;
    Real floatingBondMktFullPrice1 =
        floatingBondMktPrice1+floatingBond1->accruedAmount();
    AssetSwap floatingBondParAssetSwap1(payFixedRate,
                                        floatingBond1, floatingBondMktPrice1,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        parAssetSwap);
    floatingBondParAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondParAssetSwapSpread1 =
        floatingBondParAssetSwap1.fairSpread();
    AssetSwap floatingBondMktAssetSwap1(payFixedRate,
                                        floatingBond1, floatingBondMktPrice1,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        mktAssetSwap);
    floatingBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondMktAssetSwapSpread1 =
        floatingBondMktAssetSwap1.fairSpread();
    Real error3 =
        std::fabs(floatingBondMktAssetSwapSpread1-
                  100*floatingBondParAssetSwapSpread1/floatingBondMktFullPrice1);

    if (error3>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for floating bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(floatingBondMktAssetSwapSpread1) <<
                   "\n  par ASW spread:    " << io::rate(floatingBondParAssetSwapSpread1) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error3 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Schedule floatingBondSchedule2(Date(24,September,2004),
                                   Date(24,September,2018),
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    ext::shared_ptr<Bond> floatingBond2(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule2,
                         vars.iborIndex, Actual360(),
                         ModifiedFollowing, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004)));

    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    // market price observed on 7th June 2007
    Real floatingBondMktPrice2 = 101.248 ;
    Real floatingBondMktFullPrice2 =
        floatingBondMktPrice2+floatingBond2->accruedAmount();
    AssetSwap floatingBondParAssetSwap2(payFixedRate,
                                        floatingBond2, floatingBondMktPrice2,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        parAssetSwap);
    floatingBondParAssetSwap2.setPricingEngine(swapEngine);
    Spread floatingBondParAssetSwapSpread2 =
        floatingBondParAssetSwap2.fairSpread();
    AssetSwap floatingBondMktAssetSwap2(payFixedRate,
                                        floatingBond2, floatingBondMktPrice2,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        mktAssetSwap);
    floatingBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real floatingBondMktAssetSwapSpread2 =
        floatingBondMktAssetSwap2.fairSpread();
    Real error4 =
        std::fabs(floatingBondMktAssetSwapSpread2-
                  100*floatingBondParAssetSwapSpread2/floatingBondMktFullPrice2);

    if (error4>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for floating bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(floatingBondMktAssetSwapSpread2) <<
                   "\n  par ASW spread:    " << io::rate(floatingBondParAssetSwapSpread2) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error4 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day

    Schedule cmsBondSchedule1(Date(22,August,2005),
                              Date(22,August,2020),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond1(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule1,
                    vars.swapIndex, Thirty360(Thirty360::BondBasis),
                    Following, fixingDays,
                    std::vector<Real>(1,1.0), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(1,0.055), std::vector<Rate>(1,0.025),
                    inArrears,
                    100.0, Date(22,August,2005)));

    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondMktPrice1 = 88.45 ; // market price observed on 7th June 2007
    Real cmsBondMktFullPrice1 = cmsBondMktPrice1+cmsBond1->accruedAmount();
    AssetSwap cmsBondParAssetSwap1(payFixedRate,
                                   cmsBond1, cmsBondMktPrice1,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   parAssetSwap);
    cmsBondParAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondParAssetSwapSpread1 = cmsBondParAssetSwap1.fairSpread();
    AssetSwap cmsBondMktAssetSwap1(payFixedRate,
                                   cmsBond1, cmsBondMktPrice1,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   mktAssetSwap);
    cmsBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondMktAssetSwapSpread1 = cmsBondMktAssetSwap1.fairSpread();
    Real error5 =
        std::fabs(cmsBondMktAssetSwapSpread1-
                  100*cmsBondParAssetSwapSpread1/cmsBondMktFullPrice1);

    if (error5>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for cms bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(cmsBondMktAssetSwapSpread1) <<
                   "\n  par ASW spread:    " << io::rate(cmsBondParAssetSwapSpread1) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error5 <<
                   "\n  tolerance:         " << tolerance2);
    }

     // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
     // maturity occurs on a business day

    Schedule cmsBondSchedule2(Date(06,May,2005),
                              Date(06,May,2015),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond2(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule2,
                    vars.swapIndex, Thirty360(Thirty360::BondBasis),
                    Following, fixingDays,
                    std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(), std::vector<Rate>(),
                    inArrears,
                    100.0, Date(06,May,2005)));

    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondMktPrice2 = 94.08 ; // market price observed on 7th June 2007
    Real cmsBondMktFullPrice2 = cmsBondMktPrice2+cmsBond2->accruedAmount();
    AssetSwap cmsBondParAssetSwap2(payFixedRate,
                                   cmsBond2, cmsBondMktPrice2,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   parAssetSwap);
    cmsBondParAssetSwap2.setPricingEngine(swapEngine);
    Spread cmsBondParAssetSwapSpread2 = cmsBondParAssetSwap2.fairSpread();
    AssetSwap cmsBondMktAssetSwap2(payFixedRate,
                                   cmsBond2, cmsBondMktPrice2,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   mktAssetSwap);
    cmsBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real cmsBondMktAssetSwapSpread2 = cmsBondMktAssetSwap2.fairSpread();
    Real error6 =
        std::fabs(cmsBondMktAssetSwapSpread2-
                  100*cmsBondParAssetSwapSpread2/cmsBondMktFullPrice2);

    if (error6>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for cms bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(cmsBondMktAssetSwapSpread2) <<
                   "\n  par ASW spread:    " << io::rate(cmsBondParAssetSwapSpread2) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error6 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day

    ext::shared_ptr<Bond> zeroCpnBond1(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(20,December,2015),
                       Following,
                       100.0, Date(19,December,1985)));

    zeroCpnBond1->setPricingEngine(bondEngine);

    // market price observed on 12th June 2007
    Real zeroCpnBondMktPrice1 = 70.436 ;
    Real zeroCpnBondMktFullPrice1 =
        zeroCpnBondMktPrice1+zeroCpnBond1->accruedAmount();
    AssetSwap zeroCpnBondParAssetSwap1(payFixedRate,zeroCpnBond1,
                                       zeroCpnBondMktPrice1,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    zeroCpnBondParAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondParAssetSwapSpread1 = zeroCpnBondParAssetSwap1.fairSpread();
    AssetSwap zeroCpnBondMktAssetSwap1(payFixedRate,zeroCpnBond1,
                                       zeroCpnBondMktPrice1,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       mktAssetSwap);
    zeroCpnBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondMktAssetSwapSpread1 = zeroCpnBondMktAssetSwap1.fairSpread();
    Real error7 =
        std::fabs(zeroCpnBondMktAssetSwapSpread1-
                  100*zeroCpnBondParAssetSwapSpread1/zeroCpnBondMktFullPrice1);

    if (error7>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for zero cpn bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(zeroCpnBondMktAssetSwapSpread1) <<
                   "\n  par ASW spread:    " << io::rate(zeroCpnBondParAssetSwapSpread1) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error7 <<
                   "\n  tolerance:         " << tolerance2);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day

    ext::shared_ptr<Bond> zeroCpnBond2(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(17,February,2028),
                       Following,
                       100.0, Date(17,February,1998)));

    zeroCpnBond2->setPricingEngine(bondEngine);

    // Real zeroCpnBondPrice2 = zeroCpnBond2->cleanPrice();

    // market price observed on 12th June 2007
    Real zeroCpnBondMktPrice2 = 35.160 ;
    Real zeroCpnBondMktFullPrice2 =
        zeroCpnBondMktPrice2+zeroCpnBond2->accruedAmount();
    AssetSwap zeroCpnBondParAssetSwap2(payFixedRate,zeroCpnBond2,
                                       zeroCpnBondMktPrice2,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    zeroCpnBondParAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondParAssetSwapSpread2 = zeroCpnBondParAssetSwap2.fairSpread();
    AssetSwap zeroCpnBondMktAssetSwap2(payFixedRate,zeroCpnBond2,
                                       zeroCpnBondMktPrice2,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       mktAssetSwap);
    zeroCpnBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondMktAssetSwapSpread2 = zeroCpnBondMktAssetSwap2.fairSpread();
    Real error8 =
        std::fabs(zeroCpnBondMktAssetSwapSpread2-
                  100*zeroCpnBondParAssetSwapSpread2/zeroCpnBondMktFullPrice2);

    if (error8>tolerance2) {
        BOOST_FAIL("wrong asset swap spreads for zero cpn bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market ASW spread: " << io::rate(zeroCpnBondMktAssetSwapSpread2) <<
                   "\n  par ASW spread:    " << io::rate(zeroCpnBondParAssetSwapSpread2) <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:             " << error8 <<
                   "\n  tolerance:         " << tolerance2);
    }
}


void AssetSwapTest::testZSpread() {

    BOOST_TEST_MESSAGE("Testing clean and dirty price with null Z-spread "
                       "against theoretical prices...");

    using namespace asset_swap_test;

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool inArrears = false;

    // Fixed bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Schedule fixedBondSchedule1(Date(4,January,2005),
                                Date(4,January,2037),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond1(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule1,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005)));

    ext::shared_ptr<PricingEngine> bondEngine(
                            new DiscountingBondEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondImpliedValue1 = fixedBond1->cleanPrice();
    Date fixedBondSettlementDate1= fixedBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YC...
    Real fixedBondCleanPrice1 = BondFunctions::cleanPrice(
         *fixedBond1, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         fixedBondSettlementDate1);
    Real tolerance = 1.0e-13;
    Real error1 = std::fabs(fixedBondImpliedValue1-fixedBondCleanPrice1);
    if (error1>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market asset swap spread: " <<
                   fixedBondImpliedValue1 <<
                   "\n  par asset swap spread: " << fixedBondCleanPrice1 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error1 <<
                   "\n  tolerance:             " << tolerance);
    }

    // Fixed bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day

    Schedule fixedBondSchedule2(Date(5,February,2005),
                                Date(5,February,2019),
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    ext::shared_ptr<Bond> fixedBond2(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule2,
                      std::vector<Rate>(1, 0.05),
                      Thirty360(Thirty360::BondBasis), Following,
                      100.0, Date(5,February,2005)));

    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondImpliedValue2 = fixedBond2->cleanPrice();
    Date fixedBondSettlementDate2= fixedBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real fixedBondCleanPrice2 = BondFunctions::cleanPrice(
         *fixedBond2, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         fixedBondSettlementDate2);
    Real error3 = std::fabs(fixedBondImpliedValue2-fixedBondCleanPrice2);
    if (error3>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market asset swap spread: " <<
                   fixedBondImpliedValue2 <<
                   "\n  par asset swap spread: " << fixedBondCleanPrice2 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error3 <<
                   "\n  tolerance:             " << tolerance);
    }

    // FRN bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Schedule floatingBondSchedule1(Date(29,September,2003),
                                   Date(29,September,2013),
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);

    ext::shared_ptr<Bond> floatingBond1(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule1,
                         vars.iborIndex, Actual360(),
                         Following, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0056),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(29,September,2003)));

    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondImpliedValue1 = floatingBond1->cleanPrice();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real floatingBondCleanPrice1 = BondFunctions::cleanPrice(
        *floatingBond1, *vars.termStructure, vars.spread,
        Actual365Fixed(), vars.compounding, Semiannual,
        fixedBondSettlementDate1);
    Real error5 = std::fabs(floatingBondImpliedValue1-floatingBondCleanPrice1);
    if (error5>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:" <<
                   std::fixed << std::setprecision(4) <<
                   "\n  market asset swap spread: " <<
                   floatingBondImpliedValue1 <<
                   "\n  par asset swap spread: " << floatingBondCleanPrice1 <<
                   std::scientific << std::setprecision(2) <<
                   "\n  error:                 " << error5 <<
                   "\n  tolerance:             " << tolerance);
    }

    // FRN bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Schedule floatingBondSchedule2(Date(24,September,2004),
                                   Date(24,September,2018),
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    ext::shared_ptr<Bond> floatingBond2(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule2,
                         vars.iborIndex, Actual360(),
                         ModifiedFollowing, fixingDays,
                         std::vector<Real>(1,1), std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004)));

    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    Real floatingBondImpliedValue2 = floatingBond2->cleanPrice();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real floatingBondCleanPrice2 = BondFunctions::cleanPrice(
        *floatingBond2, *vars.termStructure,
        vars.spread, Actual365Fixed(), vars.compounding, Semiannual,
        fixedBondSettlementDate1);
    Real error7 = std::fabs(floatingBondImpliedValue2-floatingBondCleanPrice2);
    if (error7>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " <<
                    floatingBondImpliedValue2
                    << "\n  par asset swap spread: " << floatingBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance);
    }

    //// CMS bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    //// maturity doesn't occur on a business day

    Schedule cmsBondSchedule1(Date(22,August,2005),
                              Date(22,August,2020),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond1(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule1,
                    vars.swapIndex, Thirty360(Thirty360::BondBasis),
                    Following, fixingDays,
                    std::vector<Real>(1,1.0), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(1,0.055), std::vector<Rate>(1,0.025),
                    inArrears,
                    100.0, Date(22,August,2005)));

    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondImpliedValue1 = cmsBond1->cleanPrice();
    Date cmsBondSettlementDate1= cmsBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real cmsBondCleanPrice1 = BondFunctions::cleanPrice(
        *cmsBond1, *vars.termStructure, vars.spread,
        Actual365Fixed(), vars.compounding, Annual,
        cmsBondSettlementDate1);
    Real error9 = std::fabs(cmsBondImpliedValue1-cmsBondCleanPrice1);
    if (error9>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " << cmsBondImpliedValue1
                    << "\n  par asset swap spread: " << cmsBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error9
                    << "\n  tolerance:             " << tolerance);
    }

    // CMS bond (Isin: XS0218766664 ISPIM 0 5/6/15)
    // maturity occurs on a business day

    Schedule cmsBondSchedule2(Date(06,May,2005),
                              Date(06,May,2015),
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    ext::shared_ptr<Bond> cmsBond2(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule2,
                    vars.swapIndex, Thirty360(Thirty360::BondBasis),
                    Following, fixingDays,
                    std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                    std::vector<Rate>(), std::vector<Rate>(),
                    inArrears,
                    100.0, Date(06,May,2005)));

    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondImpliedValue2 = cmsBond2->cleanPrice();
    Date cmsBondSettlementDate2= cmsBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real cmsBondCleanPrice2 = BondFunctions::cleanPrice(
         *cmsBond2, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         cmsBondSettlementDate2);
    Real error11 = std::fabs(cmsBondImpliedValue2-cmsBondCleanPrice2);
    if (error11>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " << cmsBondImpliedValue2
                    << "\n  par asset swap spread: " << cmsBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error11
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero-Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day

    ext::shared_ptr<Bond> zeroCpnBond1(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(20,December,2015),
                       Following,
                       100.0, Date(19,December,1985)));

    zeroCpnBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondImpliedValue1 = zeroCpnBond1->cleanPrice();
    Date zeroCpnBondSettlementDate1= zeroCpnBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real zeroCpnBondCleanPrice1 =
        BondFunctions::cleanPrice(*zeroCpnBond1,
                              *vars.termStructure,
                              vars.spread,
                              Actual365Fixed(),
                              vars.compounding, Annual,
                              zeroCpnBondSettlementDate1);
    Real error13 = std::fabs(zeroCpnBondImpliedValue1-zeroCpnBondCleanPrice1);
    if (error13>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  zero cpn implied value: " <<
                    zeroCpnBondImpliedValue1
                    << "\n  zero cpn price: " << zeroCpnBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error13
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity doesn't occur on a business day

    ext::shared_ptr<Bond> zeroCpnBond2(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                       Date(17,February,2028),
                       Following,
                       100.0, Date(17,February,1998)));

    zeroCpnBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondImpliedValue2 = zeroCpnBond2->cleanPrice();
    Date zeroCpnBondSettlementDate2= zeroCpnBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real zeroCpnBondCleanPrice2 =
        BondFunctions::cleanPrice(*zeroCpnBond2,
                              *vars.termStructure,
                              vars.spread,
                              Actual365Fixed(),
                              vars.compounding, Annual,
                              zeroCpnBondSettlementDate2);
    Real error15 = std::fabs(zeroCpnBondImpliedValue2-zeroCpnBondCleanPrice2);
    if (error15>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  zero cpn implied value: " <<
                    zeroCpnBondImpliedValue2
                    << "\n  zero cpn price: " << zeroCpnBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error15
                    << "\n  tolerance:             " << tolerance);
    }
}


void AssetSwapTest::testGenericBondImplied() {

    BOOST_TEST_MESSAGE("Testing implied generic-bond value against"
                       " asset-swap fair price with null spread...");

    using namespace asset_swap_test;

    const auto & iborcoupon_settings = IborCoupon::Settings::instance();

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day
    Date fixedBondStartDate1 = Date(4,January,2005);
    Date fixedBondMaturityDate1 = Date(4,January,2037);
    Schedule fixedBondSchedule1(fixedBondStartDate1,
                                fixedBondMaturityDate1,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg1 = FixedRateLeg(fixedBondSchedule1)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.04, ActualActual(ActualActual::ISDA));
    Date fixedbondRedemption1 = bondCalendar.adjust(fixedBondMaturityDate1,
                                                    Following);
    fixedBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption1)));
    ext::shared_ptr<Bond> fixedBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate1, fixedBondStartDate1,
             fixedBondLeg1));
    ext::shared_ptr<PricingEngine> bondEngine(
                               new DiscountingBondEngine(vars.termStructure));
    ext::shared_ptr<PricingEngine> swapEngine(
                               new DiscountingSwapEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondPrice1 = fixedBond1->cleanPrice();
    AssetSwap fixedBondAssetSwap1(payFixedRate,
                                  fixedBond1, fixedBondPrice1,
                                  vars.iborIndex, vars.spread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice1 = fixedBondAssetSwap1.fairCleanPrice();
    Real tolerance = 1.0e-13;

    // see comment above
    Real tolerance2;
    if (!iborcoupon_settings.usingAtParCoupons())
        tolerance2 = 1.0e-2;
    else
        tolerance2 = 1.0e-13;

    Real error1 = std::fabs(fixedBondAssetSwapPrice1-fixedBondPrice1);

    if (error1>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << fixedBondPrice1
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance2);
    }

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day
    Date fixedBondStartDate2 = Date(5,February,2005);
    Date fixedBondMaturityDate2 = Date(5,February,2019);
    Schedule fixedBondSchedule2(fixedBondStartDate2,
                                fixedBondMaturityDate2,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg2 = FixedRateLeg(fixedBondSchedule2)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.05, Thirty360(Thirty360::BondBasis));
    Date fixedbondRedemption2 = bondCalendar.adjust(fixedBondMaturityDate2,
                                                    Following);
    fixedBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption2)));
    ext::shared_ptr<Bond> fixedBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate2, fixedBondStartDate2, fixedBondLeg2));
    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondPrice2 = fixedBond2->cleanPrice();
    AssetSwap fixedBondAssetSwap2(payFixedRate,
                                  fixedBond2, fixedBondPrice2,
                                  vars.iborIndex, vars.spread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice2 = fixedBondAssetSwap2.fairCleanPrice();
    Real error2 = std::fabs(fixedBondAssetSwapPrice2-fixedBondPrice2);

    if (error2>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << fixedBondPrice2
                    << "\n  asset swap fair price: " << fixedBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error2
                    << "\n  tolerance:             " << tolerance2);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day
    Date floatingBondStartDate1 = Date(29,September,2003);
    Date floatingBondMaturityDate1 = Date(29,September,2013);
    Schedule floatingBondSchedule1(floatingBondStartDate1,
                                   floatingBondMaturityDate1,
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg1 = IborLeg(floatingBondSchedule1, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withFixingDays(fixingDays)
        .withSpreads(0.0056)
        .inArrears(inArrears);
    Date floatingbondRedemption1 =
        bondCalendar.adjust(floatingBondMaturityDate1, Following);
    floatingBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption1)));
    ext::shared_ptr<Bond> floatingBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate1, floatingBondStartDate1,
             floatingBondLeg1));
    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondPrice1 = floatingBond1->cleanPrice();
    AssetSwap floatingBondAssetSwap1(payFixedRate,
                                     floatingBond1, floatingBondPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice1 = floatingBondAssetSwap1.fairCleanPrice();
    Real error3 = std::fabs(floatingBondAssetSwapPrice1-floatingBondPrice1);

    if (error3>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for floater:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << floatingBondPrice1
                    << "\n  asset swap fair price: " <<
                    floatingBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error3
                    << "\n  tolerance:             " << tolerance2);
    }

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day
    Date floatingBondStartDate2 = Date(24,September,2004);
    Date floatingBondMaturityDate2 = Date(24,September,2018);
    Schedule floatingBondSchedule2(floatingBondStartDate2,
                                   floatingBondMaturityDate2,
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg2 = IborLeg(floatingBondSchedule2, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withPaymentAdjustment(ModifiedFollowing)
        .withFixingDays(fixingDays)
        .withSpreads(0.0025)
        .inArrears(inArrears);
    Date floatingbondRedemption2 =
        bondCalendar.adjust(floatingBondMaturityDate2, ModifiedFollowing);
    floatingBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption2)));
    ext::shared_ptr<Bond> floatingBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate2, floatingBondStartDate2,
             floatingBondLeg2));
    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    Real currentCoupon=0.04013+0.0025;
    Rate floatingCurrentCoupon= floatingBond2->nextCouponRate();
    Real error4= std::fabs(floatingCurrentCoupon-currentCoupon);
    if (error4>tolerance) {
        BOOST_FAIL("wrong current coupon is returned for floater bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's calculated current coupon:      " <<
                    currentCoupon
                    << "\n  current coupon asked to the bond: " <<
                    floatingCurrentCoupon
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error4
                    << "\n  tolerance:             " << tolerance);
    }

    Real floatingBondPrice2 = floatingBond2->cleanPrice();
    AssetSwap floatingBondAssetSwap2(payFixedRate,
                                     floatingBond2, floatingBondPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap2.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice2 = floatingBondAssetSwap2.fairCleanPrice();
    Real error5 = std::fabs(floatingBondAssetSwapPrice2-floatingBondPrice2);

    if (error5>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for floater:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << floatingBondPrice2
                    << "\n  asset swap fair price: " <<
                    floatingBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error5
                    << "\n  tolerance:             " << tolerance2);
    }

    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day
    Date cmsBondStartDate1 = Date(22,August,2005);
    Date cmsBondMaturityDate1 = Date(22,August,2020);
    Schedule cmsBondSchedule1(cmsBondStartDate1,
                              cmsBondMaturityDate1,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg1 = CmsLeg(cmsBondSchedule1, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withCaps(0.055)
        .withFloors(0.025)
        .inArrears(inArrears);
    Date cmsbondRedemption1 = bondCalendar.adjust(cmsBondMaturityDate1,
                                                  Following);
    cmsBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption1)));
    ext::shared_ptr<Bond> cmsBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate1, cmsBondStartDate1, cmsBondLeg1));
    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondPrice1 = cmsBond1->cleanPrice();
    AssetSwap cmsBondAssetSwap1(payFixedRate,
                                cmsBond1, cmsBondPrice1,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice1 = cmsBondAssetSwap1.fairCleanPrice();
    Real error6 = std::fabs(cmsBondAssetSwapPrice1-cmsBondPrice1);

    if (error6>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << cmsBondPrice1
                    << "\n  asset swap fair price: " << cmsBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error6
                    << "\n  tolerance:             " << tolerance2);
    }

    // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
    // maturity occurs on a business day
    Date cmsBondStartDate2 = Date(06,May,2005);
    Date cmsBondMaturityDate2 = Date(06,May,2015);
    Schedule cmsBondSchedule2(cmsBondStartDate2,
                              cmsBondMaturityDate2,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg2 = CmsLeg(cmsBondSchedule2, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withGearings(0.84)
        .inArrears(inArrears);
    Date cmsbondRedemption2 = bondCalendar.adjust(cmsBondMaturityDate2,
                                                  Following);
    cmsBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption2)));
    ext::shared_ptr<Bond> cmsBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate2, cmsBondStartDate2, cmsBondLeg2));
    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondPrice2 = cmsBond2->cleanPrice();
    AssetSwap cmsBondAssetSwap2(payFixedRate,
                                cmsBond2, cmsBondPrice2,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap2.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice2 = cmsBondAssetSwap2.fairCleanPrice();
    Real error7 = std::fabs(cmsBondAssetSwapPrice2-cmsBondPrice2);

    if (error7>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << cmsBondPrice2
                    << "\n  asset swap fair price: " << cmsBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance2);
    }

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day
    Date zeroCpnBondStartDate1 = Date(19,December,1985);
    Date zeroCpnBondMaturityDate1 = Date(20,December,2015);
    Date zeroCpnBondRedemption1 = bondCalendar.adjust(zeroCpnBondMaturityDate1,
                                                      Following);
    Leg zeroCpnBondLeg1 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zeroCpnBondRedemption1)));
    ext::shared_ptr<Bond> zeroCpnBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate1, zeroCpnBondStartDate1, zeroCpnBondLeg1));
    zeroCpnBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice1 = zeroCpnBond1->cleanPrice();
    AssetSwap zeroCpnAssetSwap1(payFixedRate,
                                zeroCpnBond1, zeroCpnBondPrice1,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    zeroCpnAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice1 = zeroCpnAssetSwap1.fairCleanPrice();
    Real error8 = std::fabs(zeroCpnBondAssetSwapPrice1-zeroCpnBondPrice1);

    if (error8>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for zero cpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << zeroCpnBondPrice1
                    << "\n  asset swap fair price: " << zeroCpnBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error8
                    << "\n  tolerance:             " << tolerance2);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day
    Date zeroCpnBondStartDate2 = Date(17,February,1998);
    Date zeroCpnBondMaturityDate2 = Date(17,February,2028);
    Date zerocpbondRedemption2 = bondCalendar.adjust(zeroCpnBondMaturityDate2,
                                                      Following);
    Leg zeroCpnBondLeg2 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zerocpbondRedemption2)));
    ext::shared_ptr<Bond> zeroCpnBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate2, zeroCpnBondStartDate2, zeroCpnBondLeg2));
    zeroCpnBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice2 = zeroCpnBond2->cleanPrice();
    AssetSwap zeroCpnAssetSwap2(payFixedRate,
                                zeroCpnBond2, zeroCpnBondPrice2,
                                vars.iborIndex, vars.spread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    zeroCpnAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice2 = zeroCpnAssetSwap2.fairCleanPrice();
    Real error9 = std::fabs(zeroCpnBondAssetSwapPrice2-zeroCpnBondPrice2);

    if (error9>tolerance2) {
        BOOST_FAIL("wrong zero spread asset swap price for zero cpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  bond's clean price:    " << zeroCpnBondPrice2
                    << "\n  asset swap fair price: " << zeroCpnBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error9
                    << "\n  tolerance:             " << tolerance2);
    }
}


void AssetSwapTest::testMASWWithGenericBond() {

    BOOST_TEST_MESSAGE("Testing market asset swap against par asset swap "
                       "with generic bond...");

    using namespace asset_swap_test;

    const auto & iborcoupon_settings = IborCoupon::Settings::instance();

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool mktAssetSwap = false;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Date fixedBondStartDate1 = Date(4,January,2005);
    Date fixedBondMaturityDate1 = Date(4,January,2037);
    Schedule fixedBondSchedule1(fixedBondStartDate1,
                                fixedBondMaturityDate1,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg1 = FixedRateLeg(fixedBondSchedule1)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.04, ActualActual(ActualActual::ISDA));
    Date fixedbondRedemption1 = bondCalendar.adjust(fixedBondMaturityDate1,
                                                    Following);
    fixedBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption1)));
    ext::shared_ptr<Bond> fixedBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate1, fixedBondStartDate1,
             fixedBondLeg1));
    ext::shared_ptr<PricingEngine> bondEngine(
                               new DiscountingBondEngine(vars.termStructure));
    ext::shared_ptr<PricingEngine> swapEngine(
                               new DiscountingSwapEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondMktPrice1 = 89.22 ; // market price observed on 7th June 2007
    Real fixedBondMktFullPrice1=fixedBondMktPrice1+fixedBond1->accruedAmount();
    AssetSwap fixedBondParAssetSwap1(payFixedRate,
                                     fixedBond1, fixedBondMktPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    fixedBondParAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondParAssetSwapSpread1 = fixedBondParAssetSwap1.fairSpread();
    AssetSwap fixedBondMktAssetSwap1(payFixedRate,
                                     fixedBond1, fixedBondMktPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     mktAssetSwap);
    fixedBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondMktAssetSwapSpread1 = fixedBondMktAssetSwap1.fairSpread();

    // see comment above
    Real tolerance2; 
    if (!iborcoupon_settings.usingAtParCoupons())
        tolerance2 = 1.0e-4;
    else
        tolerance2 = 1.0e-13;

    Real error1 =
        std::fabs(fixedBondMktAssetSwapSpread1-
                  100*fixedBondParAssetSwapSpread1/fixedBondMktFullPrice1);

    if (error1>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for fixed bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(fixedBondMktAssetSwapSpread1) <<
                    "\n  par asset swap spread:    " << io::rate(fixedBondParAssetSwapSpread1) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error1 <<
                    "\n  tolerance:                " << tolerance2);

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day

    Date fixedBondStartDate2 = Date(5,February,2005);
    Date fixedBondMaturityDate2 = Date(5,February,2019);
    Schedule fixedBondSchedule2(fixedBondStartDate2,
                                fixedBondMaturityDate2,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg2 = FixedRateLeg(fixedBondSchedule2)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.05, Thirty360(Thirty360::BondBasis));
    Date fixedbondRedemption2 = bondCalendar.adjust(fixedBondMaturityDate2,
                                                    Following);
    fixedBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption2)));
    ext::shared_ptr<Bond> fixedBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate2, fixedBondStartDate2, fixedBondLeg2));
    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondMktPrice2 = 99.98 ; // market price observed on 7th June 2007
    Real fixedBondMktFullPrice2=fixedBondMktPrice2+fixedBond2->accruedAmount();
    AssetSwap fixedBondParAssetSwap2(payFixedRate,
                                     fixedBond2, fixedBondMktPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    fixedBondParAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondParAssetSwapSpread2 = fixedBondParAssetSwap2.fairSpread();
    AssetSwap fixedBondMktAssetSwap2(payFixedRate,
                                     fixedBond2, fixedBondMktPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     mktAssetSwap);
    fixedBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondMktAssetSwapSpread2 = fixedBondMktAssetSwap2.fairSpread();
    Real error2 =
        std::fabs(fixedBondMktAssetSwapSpread2-
                  100*fixedBondParAssetSwapSpread2/fixedBondMktFullPrice2);

    if (error2>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for fixed bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(fixedBondMktAssetSwapSpread2) <<
                    "\n  par asset swap spread:    " << io::rate(fixedBondParAssetSwapSpread2) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error2 <<
                    "\n  tolerance:                " << tolerance2);

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Date floatingBondStartDate1 = Date(29,September,2003);
    Date floatingBondMaturityDate1 = Date(29,September,2013);
    Schedule floatingBondSchedule1(floatingBondStartDate1,
                                   floatingBondMaturityDate1,
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg1 = IborLeg(floatingBondSchedule1, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withFixingDays(fixingDays)
        .withSpreads(0.0056)
        .inArrears(inArrears);
    Date floatingbondRedemption1 =
        bondCalendar.adjust(floatingBondMaturityDate1, Following);
    floatingBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption1)));
    ext::shared_ptr<Bond> floatingBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate1, floatingBondStartDate1,
             floatingBondLeg1));
    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    // market price observed on 7th June 2007
    Real floatingBondMktPrice1 = 101.64 ;
    Real floatingBondMktFullPrice1 =
        floatingBondMktPrice1+floatingBond1->accruedAmount();
    AssetSwap floatingBondParAssetSwap1(payFixedRate,
                                        floatingBond1, floatingBondMktPrice1,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        parAssetSwap);
    floatingBondParAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondParAssetSwapSpread1 =
        floatingBondParAssetSwap1.fairSpread();
    AssetSwap floatingBondMktAssetSwap1(payFixedRate,
                                        floatingBond1, floatingBondMktPrice1,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        mktAssetSwap);
    floatingBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondMktAssetSwapSpread1 =
        floatingBondMktAssetSwap1.fairSpread();
    Real error3 =
        std::fabs(floatingBondMktAssetSwapSpread1-
                  100*floatingBondParAssetSwapSpread1/floatingBondMktFullPrice1);

    if (error3>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for floating bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(floatingBondMktAssetSwapSpread1) <<
                    "\n  par asset swap spread:    " << io::rate(floatingBondParAssetSwapSpread1) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error3 <<
                    "\n  tolerance:                " << tolerance2);

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Date floatingBondStartDate2 = Date(24,September,2004);
    Date floatingBondMaturityDate2 = Date(24,September,2018);
    Schedule floatingBondSchedule2(floatingBondStartDate2,
                                   floatingBondMaturityDate2,
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg2 = IborLeg(floatingBondSchedule2, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withPaymentAdjustment(ModifiedFollowing)
        .withFixingDays(fixingDays)
        .withSpreads(0.0025)
        .inArrears(inArrears);
    Date floatingbondRedemption2 =
        bondCalendar.adjust(floatingBondMaturityDate2, ModifiedFollowing);
    floatingBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption2)));
    ext::shared_ptr<Bond> floatingBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate2, floatingBondStartDate2,
             floatingBondLeg2));
    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    // market price observed on 7th June 2007
    Real floatingBondMktPrice2 = 101.248 ;
    Real floatingBondMktFullPrice2 =
        floatingBondMktPrice2+floatingBond2->accruedAmount();
    AssetSwap floatingBondParAssetSwap2(payFixedRate,
                                        floatingBond2, floatingBondMktPrice2,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        parAssetSwap);
    floatingBondParAssetSwap2.setPricingEngine(swapEngine);
    Spread floatingBondParAssetSwapSpread2 =
        floatingBondParAssetSwap2.fairSpread();
    AssetSwap floatingBondMktAssetSwap2(payFixedRate,
                                        floatingBond2, floatingBondMktPrice2,
                                        vars.iborIndex, vars.spread,
                                        Schedule(),
                                        vars.iborIndex->dayCounter(),
                                        mktAssetSwap);
    floatingBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real floatingBondMktAssetSwapSpread2 =
        floatingBondMktAssetSwap2.fairSpread();
    Real error4 =
        std::fabs(floatingBondMktAssetSwapSpread2-
                  100*floatingBondParAssetSwapSpread2/floatingBondMktFullPrice2);

    if (error4>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for floating bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(floatingBondMktAssetSwapSpread2) <<
                    "\n  par asset swap spread:    " << io::rate(floatingBondParAssetSwapSpread2) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error4 <<
                    "\n  tolerance:                " << tolerance2);

    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day

    Date cmsBondStartDate1 = Date(22,August,2005);
    Date cmsBondMaturityDate1 = Date(22,August,2020);
    Schedule cmsBondSchedule1(cmsBondStartDate1,
                              cmsBondMaturityDate1,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg1 = CmsLeg(cmsBondSchedule1, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withCaps(0.055)
        .withFloors(0.025)
        .inArrears(inArrears);
    Date cmsbondRedemption1 = bondCalendar.adjust(cmsBondMaturityDate1,
                                                  Following);
    cmsBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption1)));
    ext::shared_ptr<Bond> cmsBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate1, cmsBondStartDate1, cmsBondLeg1));
    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondMktPrice1 = 88.45 ; // market price observed on 7th June 2007
    Real cmsBondMktFullPrice1 = cmsBondMktPrice1+cmsBond1->accruedAmount();
    AssetSwap cmsBondParAssetSwap1(payFixedRate,
                                   cmsBond1, cmsBondMktPrice1,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   parAssetSwap);
    cmsBondParAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondParAssetSwapSpread1 = cmsBondParAssetSwap1.fairSpread();
    AssetSwap cmsBondMktAssetSwap1(payFixedRate,
                                   cmsBond1, cmsBondMktPrice1,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   mktAssetSwap);
    cmsBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondMktAssetSwapSpread1 = cmsBondMktAssetSwap1.fairSpread();
    Real error5 =
        std::fabs(cmsBondMktAssetSwapSpread1-
                  100*cmsBondParAssetSwapSpread1/cmsBondMktFullPrice1);

    if (error5>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for cms bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(cmsBondMktAssetSwapSpread1) <<
                    "\n  par asset swap spread:    " << io::rate(cmsBondParAssetSwapSpread1) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error5 <<
                    "\n  tolerance:                " << tolerance2);

     // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
     // maturity occurs on a business day

    Date cmsBondStartDate2 = Date(06,May,2005);
    Date cmsBondMaturityDate2 = Date(06,May,2015);
    Schedule cmsBondSchedule2(cmsBondStartDate2,
                              cmsBondMaturityDate2,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg2 = CmsLeg(cmsBondSchedule2, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withGearings(0.84)
        .inArrears(inArrears);
    Date cmsbondRedemption2 = bondCalendar.adjust(cmsBondMaturityDate2,
                                                  Following);
    cmsBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption2)));
    ext::shared_ptr<Bond> cmsBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate2, cmsBondStartDate2, cmsBondLeg2));
    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondMktPrice2 = 94.08 ; // market price observed on 7th June 2007
    Real cmsBondMktFullPrice2 = cmsBondMktPrice2+cmsBond2->accruedAmount();
    AssetSwap cmsBondParAssetSwap2(payFixedRate,
                                   cmsBond2, cmsBondMktPrice2,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   parAssetSwap);
    cmsBondParAssetSwap2.setPricingEngine(swapEngine);
    Spread cmsBondParAssetSwapSpread2 = cmsBondParAssetSwap2.fairSpread();
    AssetSwap cmsBondMktAssetSwap2(payFixedRate,
                                   cmsBond2, cmsBondMktPrice2,
                                   vars.iborIndex, vars.spread,
                                   Schedule(),
                                   vars.iborIndex->dayCounter(),
                                   mktAssetSwap);
    cmsBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real cmsBondMktAssetSwapSpread2 = cmsBondMktAssetSwap2.fairSpread();
    Real error6 =
        std::fabs(cmsBondMktAssetSwapSpread2-
                  100*cmsBondParAssetSwapSpread2/cmsBondMktFullPrice2);

    if (error6>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for cms bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(cmsBondMktAssetSwapSpread2) <<
                    "\n  par asset swap spread:    " << io::rate(cmsBondParAssetSwapSpread2) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error6 <<
                    "\n  tolerance:                " << tolerance2);

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day

    Date zeroCpnBondStartDate1 = Date(19,December,1985);
    Date zeroCpnBondMaturityDate1 = Date(20,December,2015);
    Date zeroCpnBondRedemption1 = bondCalendar.adjust(zeroCpnBondMaturityDate1,
                                                      Following);
    Leg zeroCpnBondLeg1 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zeroCpnBondRedemption1)));
    ext::shared_ptr<Bond> zeroCpnBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate1, zeroCpnBondStartDate1, zeroCpnBondLeg1));
    zeroCpnBond1->setPricingEngine(bondEngine);

    // market price observed on 12th June 2007
    Real zeroCpnBondMktPrice1 = 70.436 ;
    Real zeroCpnBondMktFullPrice1 =
        zeroCpnBondMktPrice1+zeroCpnBond1->accruedAmount();
    AssetSwap zeroCpnBondParAssetSwap1(payFixedRate,zeroCpnBond1,
                                       zeroCpnBondMktPrice1,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    zeroCpnBondParAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondParAssetSwapSpread1 = zeroCpnBondParAssetSwap1.fairSpread();
    AssetSwap zeroCpnBondMktAssetSwap1(payFixedRate,zeroCpnBond1,
                                       zeroCpnBondMktPrice1,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       mktAssetSwap);
    zeroCpnBondMktAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondMktAssetSwapSpread1 = zeroCpnBondMktAssetSwap1.fairSpread();
    Real error7 =
        std::fabs(zeroCpnBondMktAssetSwapSpread1-
                  100*zeroCpnBondParAssetSwapSpread1/zeroCpnBondMktFullPrice1);

    if (error7>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for zero cpn bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(zeroCpnBondMktAssetSwapSpread1) <<
                    "\n  par asset swap spread:    " << io::rate(zeroCpnBondParAssetSwapSpread1) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error7 <<
                    "\n  tolerance:                " << tolerance2);

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day

    Date zeroCpnBondStartDate2 = Date(17,February,1998);
    Date zeroCpnBondMaturityDate2 = Date(17,February,2028);
    Date zerocpbondRedemption2 = bondCalendar.adjust(zeroCpnBondMaturityDate2,
                                                      Following);
    Leg zeroCpnBondLeg2 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zerocpbondRedemption2)));
    ext::shared_ptr<Bond> zeroCpnBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate2, zeroCpnBondStartDate2, zeroCpnBondLeg2));
    zeroCpnBond2->setPricingEngine(bondEngine);

    // Real zeroCpnBondPrice2 = zeroCpnBond2->cleanPrice();
    // market price observed on 12th June 2007
    Real zeroCpnBondMktPrice2 = 35.160 ;
    Real zeroCpnBondMktFullPrice2 =
        zeroCpnBondMktPrice2+zeroCpnBond2->accruedAmount();
    AssetSwap zeroCpnBondParAssetSwap2(payFixedRate,zeroCpnBond2,
                                       zeroCpnBondMktPrice2,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    zeroCpnBondParAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondParAssetSwapSpread2 = zeroCpnBondParAssetSwap2.fairSpread();
    AssetSwap zeroCpnBondMktAssetSwap2(payFixedRate,zeroCpnBond2,
                                       zeroCpnBondMktPrice2,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       mktAssetSwap);
    zeroCpnBondMktAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondMktAssetSwapSpread2 = zeroCpnBondMktAssetSwap2.fairSpread();
    Real error8 =
        std::fabs(zeroCpnBondMktAssetSwapSpread2-
                  100*zeroCpnBondParAssetSwapSpread2/zeroCpnBondMktFullPrice2);

    if (error8>tolerance2)
        BOOST_FAIL("wrong asset swap spreads for zero cpn bond:" <<
                    std::fixed << std::setprecision(4) <<
                    "\n  market asset swap spread: " << io::rate(zeroCpnBondMktAssetSwapSpread2) <<
                    "\n  par asset swap spread:    " << io::rate(zeroCpnBondParAssetSwapSpread2) <<
                    std::scientific << std::setprecision(2) <<
                    "\n  error:                    " << error8 <<
                    "\n  tolerance:                " << tolerance2);
}


void AssetSwapTest::testZSpreadWithGenericBond() {

    BOOST_TEST_MESSAGE("Testing clean and dirty price with null Z-spread "
                       "against theoretical prices...");

    using namespace asset_swap_test;

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day

    Date fixedBondStartDate1 = Date(4,January,2005);
    Date fixedBondMaturityDate1 = Date(4,January,2037);
    Schedule fixedBondSchedule1(fixedBondStartDate1,
                                fixedBondMaturityDate1,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg1 = FixedRateLeg(fixedBondSchedule1)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.04, ActualActual(ActualActual::ISDA));
    Date fixedbondRedemption1 = bondCalendar.adjust(fixedBondMaturityDate1,
                                                    Following);
    fixedBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption1)));
    ext::shared_ptr<Bond> fixedBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate1, fixedBondStartDate1,
             fixedBondLeg1));
    ext::shared_ptr<PricingEngine> bondEngine(new
        DiscountingBondEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    Real fixedBondImpliedValue1 = fixedBond1->cleanPrice();
    Date fixedBondSettlementDate1= fixedBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real fixedBondCleanPrice1 = BondFunctions::cleanPrice(
         *fixedBond1, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         fixedBondSettlementDate1);
    Real tolerance = 1.0e-13;
    Real error1 = std::fabs(fixedBondImpliedValue1-fixedBondCleanPrice1);
    if (error1>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: "
                    << fixedBondImpliedValue1
                    << "\n  par asset swap spread: " << fixedBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance);
    }

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day

    Date fixedBondStartDate2 = Date(5,February,2005);
    Date fixedBondMaturityDate2 = Date(5,February,2019);
    Schedule fixedBondSchedule2(fixedBondStartDate2,
                                fixedBondMaturityDate2,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg2 = FixedRateLeg(fixedBondSchedule2)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.05, Thirty360(Thirty360::BondBasis));
    Date fixedbondRedemption2 = bondCalendar.adjust(fixedBondMaturityDate2,
                                                    Following);
    fixedBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption2)));
    ext::shared_ptr<Bond> fixedBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate2, fixedBondStartDate2, fixedBondLeg2));
    fixedBond2->setPricingEngine(bondEngine);

    Real fixedBondImpliedValue2 = fixedBond2->cleanPrice();
    Date fixedBondSettlementDate2= fixedBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve

    Real fixedBondCleanPrice2 = BondFunctions::cleanPrice(
         *fixedBond2, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         fixedBondSettlementDate2);
    Real error3 = std::fabs(fixedBondImpliedValue2-fixedBondCleanPrice2);
    if (error3>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: "
                    << fixedBondImpliedValue2
                    << "\n  par asset swap spread: " << fixedBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error3
                    << "\n  tolerance:             " << tolerance);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day

    Date floatingBondStartDate1 = Date(29,September,2003);
    Date floatingBondMaturityDate1 = Date(29,September,2013);
    Schedule floatingBondSchedule1(floatingBondStartDate1,
                                   floatingBondMaturityDate1,
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg1 = IborLeg(floatingBondSchedule1, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withFixingDays(fixingDays)
        .withSpreads(0.0056)
        .inArrears(inArrears);
    Date floatingbondRedemption1 =
        bondCalendar.adjust(floatingBondMaturityDate1, Following);
    floatingBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption1)));
    ext::shared_ptr<Bond> floatingBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate1, floatingBondStartDate1,
             floatingBondLeg1));
    floatingBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondImpliedValue1 = floatingBond1->cleanPrice();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real floatingBondCleanPrice1 = BondFunctions::cleanPrice(
        *floatingBond1, *vars.termStructure,
        vars.spread, Actual365Fixed(), vars.compounding, Semiannual,
        fixedBondSettlementDate1);
    Real error5 = std::fabs(floatingBondImpliedValue1-floatingBondCleanPrice1);
    if (error5>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " <<
                    floatingBondImpliedValue1
                    << "\n  par asset swap spread: " << floatingBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error5
                    << "\n  tolerance:             " << tolerance);
    }

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day

    Date floatingBondStartDate2 = Date(24,September,2004);
    Date floatingBondMaturityDate2 = Date(24,September,2018);
    Schedule floatingBondSchedule2(floatingBondStartDate2,
                                   floatingBondMaturityDate2,
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg2 = IborLeg(floatingBondSchedule2, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withPaymentAdjustment(ModifiedFollowing)
        .withFixingDays(fixingDays)
        .withSpreads(0.0025)
        .inArrears(inArrears);
    Date floatingbondRedemption2 =
        bondCalendar.adjust(floatingBondMaturityDate2, ModifiedFollowing);
    floatingBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption2)));
    ext::shared_ptr<Bond> floatingBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate2, floatingBondStartDate2,
             floatingBondLeg2));
    floatingBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);
    Real floatingBondImpliedValue2 = floatingBond2->cleanPrice();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real floatingBondCleanPrice2 = BondFunctions::cleanPrice(
        *floatingBond2, *vars.termStructure,
        vars.spread, Actual365Fixed(), vars.compounding, Semiannual,
        fixedBondSettlementDate1);
    Real error7 = std::fabs(floatingBondImpliedValue2-floatingBondCleanPrice2);
    if (error7>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " <<
                    floatingBondImpliedValue2
                    << "\n  par asset swap spread: " << floatingBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance);
    }

    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day

    Date cmsBondStartDate1 = Date(22,August,2005);
    Date cmsBondMaturityDate1 = Date(22,August,2020);
    Schedule cmsBondSchedule1(cmsBondStartDate1,
                              cmsBondMaturityDate1,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg1 = CmsLeg(cmsBondSchedule1, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withCaps(0.055)
        .withFloors(0.025)
        .inArrears(inArrears);
    Date cmsbondRedemption1 = bondCalendar.adjust(cmsBondMaturityDate1,
                                                  Following);
    cmsBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption1)));
    ext::shared_ptr<Bond> cmsBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate1, cmsBondStartDate1, cmsBondLeg1));
    cmsBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondImpliedValue1 = cmsBond1->cleanPrice();
    Date cmsBondSettlementDate1= cmsBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real cmsBondCleanPrice1 = BondFunctions::cleanPrice(
         *cmsBond1, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         cmsBondSettlementDate1);
    Real error9 = std::fabs(cmsBondImpliedValue1-cmsBondCleanPrice1);
    if (error9>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " << cmsBondImpliedValue1
                    << "\n  par asset swap spread: " << cmsBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error9
                    << "\n  tolerance:             " << tolerance);
    }

    // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
    // maturity occurs on a business day

    Date cmsBondStartDate2 = Date(06,May,2005);
    Date cmsBondMaturityDate2 = Date(06,May,2015);
    Schedule cmsBondSchedule2(cmsBondStartDate2,
                              cmsBondMaturityDate2,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg2 = CmsLeg(cmsBondSchedule2, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withGearings(0.84)
        .inArrears(inArrears);
    Date cmsbondRedemption2 = bondCalendar.adjust(cmsBondMaturityDate2,
                                                  Following);
    cmsBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption2)));
    ext::shared_ptr<Bond> cmsBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate2, cmsBondStartDate2, cmsBondLeg2));
    cmsBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondImpliedValue2 = cmsBond2->cleanPrice();
    Date cmsBondSettlementDate2= cmsBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real cmsBondCleanPrice2 = BondFunctions::cleanPrice(
         *cmsBond2, *vars.termStructure, vars.spread,
         Actual365Fixed(), vars.compounding, Annual,
         cmsBondSettlementDate2);
    Real error11 = std::fabs(cmsBondImpliedValue2-cmsBondCleanPrice2);
    if (error11>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  market asset swap spread: " << cmsBondImpliedValue2
                    << "\n  par asset swap spread: " << cmsBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error11
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day

    Date zeroCpnBondStartDate1 = Date(19,December,1985);
    Date zeroCpnBondMaturityDate1 = Date(20,December,2015);
    Date zeroCpnBondRedemption1 = bondCalendar.adjust(zeroCpnBondMaturityDate1,
                                                      Following);
    Leg zeroCpnBondLeg1 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zeroCpnBondRedemption1)));
    ext::shared_ptr<Bond> zeroCpnBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate1, zeroCpnBondStartDate1, zeroCpnBondLeg1));
    zeroCpnBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondImpliedValue1 = zeroCpnBond1->cleanPrice();
    Date zeroCpnBondSettlementDate1= zeroCpnBond1->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real zeroCpnBondCleanPrice1 =
        BondFunctions::cleanPrice(*zeroCpnBond1,
                              *vars.termStructure,
                              vars.spread,
                              Actual365Fixed(),
                              vars.compounding, Annual,
                              zeroCpnBondSettlementDate1);
    Real error13 = std::fabs(zeroCpnBondImpliedValue1-zeroCpnBondCleanPrice1);
    if (error13>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  zero cpn implied value: " <<
                    zeroCpnBondImpliedValue1
                    << "\n  zero cpn price: " << zeroCpnBondCleanPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error13
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day

    Date zeroCpnBondStartDate2 = Date(17,February,1998);
    Date zeroCpnBondMaturityDate2 = Date(17,February,2028);
    Date zerocpbondRedemption2 = bondCalendar.adjust(zeroCpnBondMaturityDate2,
                                                      Following);
    Leg zeroCpnBondLeg2 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zerocpbondRedemption2)));
    ext::shared_ptr<Bond> zeroCpnBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate2, zeroCpnBondStartDate2, zeroCpnBondLeg2));
    zeroCpnBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondImpliedValue2 = zeroCpnBond2->cleanPrice();
    Date zeroCpnBondSettlementDate2= zeroCpnBond2->settlementDate();
    // standard market conventions:
    // bond's frequency + coumpounding and daycounter of the YieldCurve
    Real zeroCpnBondCleanPrice2 =
        BondFunctions::cleanPrice(*zeroCpnBond2,
                              *vars.termStructure,
                              vars.spread,
                              Actual365Fixed(),
                              vars.compounding, Annual,
                              zeroCpnBondSettlementDate2);
    Real error15 = std::fabs(zeroCpnBondImpliedValue2-zeroCpnBondCleanPrice2);
    if (error15>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  zero cpn implied value: " <<
                    zeroCpnBondImpliedValue2
                    << "\n  zero cpn price: " << zeroCpnBondCleanPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error15
                    << "\n  tolerance:             " << tolerance);
    }
}


void AssetSwapTest::testSpecializedBondVsGenericBond() {

    BOOST_TEST_MESSAGE("Testing clean and dirty prices for specialized bond"
                       " against equivalent generic bond...");

    using namespace asset_swap_test;

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool inArrears = false;

    // Fixed Underlying bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day
    Date fixedBondStartDate1 = Date(4,January,2005);
    Date fixedBondMaturityDate1 = Date(4,January,2037);
    Schedule fixedBondSchedule1(fixedBondStartDate1,
                                fixedBondMaturityDate1,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg1 = FixedRateLeg(fixedBondSchedule1)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.04, ActualActual(ActualActual::ISDA));
    Date fixedbondRedemption1 = bondCalendar.adjust(fixedBondMaturityDate1,
                                                    Following);
    fixedBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> fixedBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate1, fixedBondStartDate1,
             fixedBondLeg1));
    ext::shared_ptr<PricingEngine> bondEngine(new
        DiscountingBondEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    // equivalent specialized fixed rate bond
    ext::shared_ptr<Bond> fixedSpecializedBond1(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule1,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005) ));
    fixedSpecializedBond1->setPricingEngine(bondEngine);

    Real fixedBondTheoValue1 = fixedBond1->cleanPrice();
    Real fixedSpecializedBondTheoValue1 = fixedSpecializedBond1->cleanPrice();
    Real tolerance = 1.0e-13;
    Real error1 = std::fabs(fixedBondTheoValue1-fixedSpecializedBondTheoValue1);
    if (error1>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  specialized fixed rate bond's theo clean price: "
                    << fixedBondTheoValue1
                    << "\n  generic equivalent bond's theo clean price: "
                    << fixedSpecializedBondTheoValue1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance);
    }
    Real fixedBondTheoDirty1 = fixedBondTheoValue1+fixedBond1->accruedAmount();
    Real fixedSpecializedTheoDirty1 = fixedSpecializedBondTheoValue1+
                                  fixedSpecializedBond1->accruedAmount();
    Real error2 = std::fabs(fixedBondTheoDirty1-fixedSpecializedTheoDirty1);
    if (error2>tolerance) {
        BOOST_FAIL("wrong dirty price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  specialized fixed rate bond's theo dirty price: "
                    << fixedBondTheoDirty1
                    << "\n  generic equivalent bond's theo dirty price: "
                    << fixedSpecializedTheoDirty1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error2
                    << "\n  tolerance:             " << tolerance);
    }

    // Fixed Underlying bond (Isin: IT0006527060 IBRD 5 02/05/19)
    // maturity occurs on a business day
    Date fixedBondStartDate2 = Date(5,February,2005);
    Date fixedBondMaturityDate2 = Date(5,February,2019);
    Schedule fixedBondSchedule2(fixedBondStartDate2,
                                fixedBondMaturityDate2,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg2 = FixedRateLeg(fixedBondSchedule2)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.05, Thirty360(Thirty360::BondBasis));
    Date fixedbondRedemption2 = bondCalendar.adjust(fixedBondMaturityDate2,
                                                    Following);
    fixedBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption2)));

    // generic bond
    ext::shared_ptr<Bond> fixedBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate2, fixedBondStartDate2, fixedBondLeg2));
    fixedBond2->setPricingEngine(bondEngine);

    // equivalent specialized fixed rate bond
    ext::shared_ptr<Bond> fixedSpecializedBond2(new
         FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule2,
                      std::vector<Rate>(1, 0.05),
                      Thirty360(Thirty360::BondBasis), Following,
                      100.0, Date(5,February,2005)));
    fixedSpecializedBond2->setPricingEngine(bondEngine);

    Real fixedBondTheoValue2 = fixedBond2->cleanPrice();
    Real fixedSpecializedBondTheoValue2 = fixedSpecializedBond2->cleanPrice();

    Real error3 = std::fabs(fixedBondTheoValue2-fixedSpecializedBondTheoValue2);
    if (error3>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  specialized fixed rate bond's theo clean price: "
                    << fixedBondTheoValue2
                    << "\n  generic equivalent bond's theo clean price: "
                    << fixedSpecializedBondTheoValue2
                    << "\n  error:                 " << error3
                    << "\n  tolerance:             " << tolerance);
    }
    Real fixedBondTheoDirty2 = fixedBondTheoValue2+
                               fixedBond2->accruedAmount();
    Real fixedSpecializedBondTheoDirty2 = fixedSpecializedBondTheoValue2+
                                      fixedSpecializedBond2->accruedAmount();

    Real error4 = std::fabs(fixedBondTheoDirty2-fixedSpecializedBondTheoDirty2);
    if (error4>tolerance) {
        BOOST_FAIL("wrong dirty price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  specialized fixed rate bond's dirty clean price: "
                    << fixedBondTheoDirty2
                    << "\n  generic equivalent bond's theo dirty price: "
                    << fixedSpecializedBondTheoDirty2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error4
                    << "\n  tolerance:             " << tolerance);
    }

    // FRN Underlying bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    // maturity doesn't occur on a business day
    Date floatingBondStartDate1 = Date(29,September,2003);
    Date floatingBondMaturityDate1 = Date(29,September,2013);
    Schedule floatingBondSchedule1(floatingBondStartDate1,
                                   floatingBondMaturityDate1,
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg1 = IborLeg(floatingBondSchedule1, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withFixingDays(fixingDays)
        .withSpreads(0.0056)
        .inArrears(inArrears);
    Date floatingbondRedemption1 =
        bondCalendar.adjust(floatingBondMaturityDate1, Following);
    floatingBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> floatingBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate1, floatingBondStartDate1,
             floatingBondLeg1));
    floatingBond1->setPricingEngine(bondEngine);

    // equivalent specialized floater
    ext::shared_ptr<Bond> floatingSpecializedBond1(new
           FloatingRateBond(settlementDays, vars.faceAmount,
                            floatingBondSchedule1,
                            vars.iborIndex, Actual360(),
                            Following, fixingDays,
                            std::vector<Real>(1,1),
                            std::vector<Spread>(1,0.0056),
                            std::vector<Rate>(), std::vector<Rate>(),
                            inArrears,
                            100.0, Date(29,September,2003)));
    floatingSpecializedBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    setCouponPricer(floatingSpecializedBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondTheoValue1 = floatingBond1->cleanPrice();
    Real floatingSpecializedBondTheoValue1 =
        floatingSpecializedBond1->cleanPrice();

    Real error5 = std::fabs(floatingBondTheoValue1-
                            floatingSpecializedBondTheoValue1);
    if (error5>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic fixed rate bond's theo clean price: "
                    << floatingBondTheoValue1
                    << "\n  equivalent specialized bond's theo clean price: "
                    << floatingSpecializedBondTheoValue1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error5
                    << "\n  tolerance:             " << tolerance);
    }
    Real floatingBondTheoDirty1 = floatingBondTheoValue1+
                                  floatingBond1->accruedAmount();
    Real floatingSpecializedBondTheoDirty1 =
        floatingSpecializedBondTheoValue1+
        floatingSpecializedBond1->accruedAmount();
    Real error6 = std::fabs(floatingBondTheoDirty1-
                            floatingSpecializedBondTheoDirty1);
    if (error6>tolerance) {
        BOOST_FAIL("wrong dirty price for frn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic frn bond's dirty clean price: "
                    << floatingBondTheoDirty1
                    << "\n  equivalent specialized bond's theo dirty price: "
                    << floatingSpecializedBondTheoDirty1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error6
                    << "\n  tolerance:             " << tolerance);
    }

    // FRN Underlying bond (Isin: XS0090566539 COE 0 09/24/18)
    // maturity occurs on a business day
    Date floatingBondStartDate2 = Date(24,September,2004);
    Date floatingBondMaturityDate2 = Date(24,September,2018);
    Schedule floatingBondSchedule2(floatingBondStartDate2,
                                   floatingBondMaturityDate2,
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg2 = IborLeg(floatingBondSchedule2, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withPaymentAdjustment(ModifiedFollowing)
        .withFixingDays(fixingDays)
        .withSpreads(0.0025)
        .inArrears(inArrears);
    Date floatingbondRedemption2 =
        bondCalendar.adjust(floatingBondMaturityDate2, ModifiedFollowing);
    floatingBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> floatingBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate2, floatingBondStartDate2,
             floatingBondLeg2));
    floatingBond2->setPricingEngine(bondEngine);

    // equivalent specialized floater
    ext::shared_ptr<Bond> floatingSpecializedBond2(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule2,
                         vars.iborIndex, Actual360(),
                         ModifiedFollowing, fixingDays,
                         std::vector<Real>(1,1),
                         std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004)));
    floatingSpecializedBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    setCouponPricer(floatingSpecializedBond2->cashflows(), vars.pricer);

    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);

    Real floatingBondTheoValue2 = floatingBond2->cleanPrice();
    Real floatingSpecializedBondTheoValue2 =
        floatingSpecializedBond2->cleanPrice();

    Real error7 =
        std::fabs(floatingBondTheoValue2-floatingSpecializedBondTheoValue2);
    if (error7>tolerance) {
        BOOST_FAIL("wrong clean price for floater bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic floater bond's theo clean price: "
                    << floatingBondTheoValue2
                    << "\n  equivalent specialized bond's theo clean price: "
                    << floatingSpecializedBondTheoValue2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance);
    }
    Real floatingBondTheoDirty2 = floatingBondTheoValue2+
                                  floatingBond2->accruedAmount();
    Real floatingSpecializedTheoDirty2 = floatingSpecializedBondTheoValue2+
                                     floatingSpecializedBond2->accruedAmount();

    Real error8 =
        std::fabs(floatingBondTheoDirty2-floatingSpecializedTheoDirty2);
    if (error8>tolerance) {
        BOOST_FAIL("wrong dirty price for floater bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic floater bond's theo dirty price: "
                    << floatingBondTheoDirty2
                    << "\n  equivalent specialized  bond's theo dirty price: "
                    << floatingSpecializedTheoDirty2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error8
                    << "\n  tolerance:             " << tolerance);
    }


    // CMS Underlying bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day
    Date cmsBondStartDate1 = Date(22,August,2005);
    Date cmsBondMaturityDate1 = Date(22,August,2020);
    Schedule cmsBondSchedule1(cmsBondStartDate1,
                              cmsBondMaturityDate1,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg1 = CmsLeg(cmsBondSchedule1, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withCaps(0.055)
        .withFloors(0.025)
        .inArrears(inArrears);
    Date cmsbondRedemption1 = bondCalendar.adjust(cmsBondMaturityDate1,
                                                  Following);
    cmsBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption1)));
    // generic cms bond
    ext::shared_ptr<Bond> cmsBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate1, cmsBondStartDate1, cmsBondLeg1));
    cmsBond1->setPricingEngine(bondEngine);

    // equivalent specialized cms bond
    ext::shared_ptr<Bond> cmsSpecializedBond1(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule1,
                vars.swapIndex, Thirty360(Thirty360::BondBasis),
                Following, fixingDays,
                std::vector<Real>(1,1.0), std::vector<Spread>(1,0.0),
                std::vector<Rate>(1,0.055), std::vector<Rate>(1,0.025),
                inArrears,
                100.0, Date(22,August,2005)));
    cmsSpecializedBond1->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    setCouponPricer(cmsSpecializedBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondTheoValue1 = cmsBond1->cleanPrice();
    Real cmsSpecializedBondTheoValue1 = cmsSpecializedBond1->cleanPrice();
    Real error9 = std::fabs(cmsBondTheoValue1-cmsSpecializedBondTheoValue1);
    if (error9>tolerance) {
        BOOST_FAIL("wrong clean price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic cms bond's theo clean price: "
                    << cmsBondTheoValue1
                    <<  "\n  equivalent specialized bond's theo clean price: "
                    << cmsSpecializedBondTheoValue1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error9
                    << "\n  tolerance:             " << tolerance);
    }
    Real cmsBondTheoDirty1 = cmsBondTheoValue1+cmsBond1->accruedAmount();
    Real cmsSpecializedBondTheoDirty1 = cmsSpecializedBondTheoValue1+
                                    cmsSpecializedBond1->accruedAmount();
    Real error10 = std::fabs(cmsBondTheoDirty1-cmsSpecializedBondTheoDirty1);
    if (error10>tolerance) {
        BOOST_FAIL("wrong dirty price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n generic cms bond's theo dirty price: "
                    << cmsBondTheoDirty1
                    << "\n  specialized cms bond's theo dirty price: "
                    << cmsSpecializedBondTheoDirty1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error10
                    << "\n  tolerance:             " << tolerance);
    }

    // CMS Underlying bond (Isin: XS0218766664 ISPIM 0 5/6/15)
    // maturity occurs on a business day
    Date cmsBondStartDate2 = Date(06,May,2005);
    Date cmsBondMaturityDate2 = Date(06,May,2015);
    Schedule cmsBondSchedule2(cmsBondStartDate2,
                              cmsBondMaturityDate2,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg2 = CmsLeg(cmsBondSchedule2, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withGearings(0.84)
        .inArrears(inArrears);
    Date cmsbondRedemption2 = bondCalendar.adjust(cmsBondMaturityDate2,
                                                  Following);
    cmsBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> cmsBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate2, cmsBondStartDate2, cmsBondLeg2));
    cmsBond2->setPricingEngine(bondEngine);

    // equivalent specialized cms bond
    ext::shared_ptr<Bond> cmsSpecializedBond2(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule2,
                vars.swapIndex, Thirty360(Thirty360::BondBasis),
                Following, fixingDays,
                std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                std::vector<Rate>(), std::vector<Rate>(),
                inArrears,
                100.0, Date(06,May,2005)));
    cmsSpecializedBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    setCouponPricer(cmsSpecializedBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondTheoValue2 = cmsBond2->cleanPrice();
    Real cmsSpecializedBondTheoValue2 = cmsSpecializedBond2->cleanPrice();

    Real error11 = std::fabs(cmsBondTheoValue2-cmsSpecializedBondTheoValue2);
    if (error11>tolerance) {
        BOOST_FAIL("wrong clean price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic cms bond's theo clean price: "
                    << cmsBondTheoValue2
                    << "\n  cms bond's theo clean price: "
                    << cmsSpecializedBondTheoValue2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error11
                    << "\n  tolerance:             " << tolerance);
    }
    Real cmsBondTheoDirty2 = cmsBondTheoValue2+cmsBond2->accruedAmount();
    Real cmsSpecializedBondTheoDirty2 =
        cmsSpecializedBondTheoValue2+cmsSpecializedBond2->accruedAmount();
    Real error12 = std::fabs(cmsBondTheoDirty2-cmsSpecializedBondTheoDirty2);
    if (error12>tolerance) {
        BOOST_FAIL("wrong dirty price for cms bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic cms bond's dirty price: "
                    << cmsBondTheoDirty2
                    << "\n  specialized cms bond's theo dirty price: "
                    << cmsSpecializedBondTheoDirty2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error12
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
    // maturity doesn't occur on a business day
    Date zeroCpnBondStartDate1 = Date(19,December,1985);
    Date zeroCpnBondMaturityDate1 = Date(20,December,2015);
    Date zeroCpnBondRedemption1 = bondCalendar.adjust(zeroCpnBondMaturityDate1,
                                                      Following);
    Leg zeroCpnBondLeg1 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zeroCpnBondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> zeroCpnBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate1, zeroCpnBondStartDate1, zeroCpnBondLeg1));
    zeroCpnBond1->setPricingEngine(bondEngine);

    // specialized zerocpn bond
    ext::shared_ptr<Bond> zeroCpnSpecializedBond1(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                  Date(20,December,2015),
                  Following,
                  100.0, Date(19,December,1985)));
    zeroCpnSpecializedBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondTheoValue1 = zeroCpnBond1->cleanPrice();
    Real zeroCpnSpecializedBondTheoValue1 =
        zeroCpnSpecializedBond1->cleanPrice();

    Real error13 =
        std::fabs(zeroCpnBondTheoValue1-zeroCpnSpecializedBondTheoValue1);
    if (error13>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zero bond's clean price: "
                    << zeroCpnBondTheoValue1
                    << "\n  specialized zero bond's clean price: "
                    << zeroCpnSpecializedBondTheoValue1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error13
                    << "\n  tolerance:             " << tolerance);
    }
    Real zeroCpnBondTheoDirty1 = zeroCpnBondTheoValue1+
                                 zeroCpnBond1->accruedAmount();
    Real zeroCpnSpecializedBondTheoDirty1 =
        zeroCpnSpecializedBondTheoValue1+
        zeroCpnSpecializedBond1->accruedAmount();
    Real error14 =
        std::fabs(zeroCpnBondTheoDirty1-zeroCpnSpecializedBondTheoDirty1);
    if (error14>tolerance) {
        BOOST_FAIL("wrong dirty price for zero bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zerocpn bond's dirty price: "
                    << zeroCpnBondTheoDirty1
                    << "\n  specialized zerocpn bond's clean price: "
                    << zeroCpnSpecializedBondTheoDirty1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error14
                    << "\n  tolerance:             " << tolerance);
    }

    // Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
    // maturity occurs on a business day
    Date zeroCpnBondStartDate2 = Date(17,February,1998);
    Date zeroCpnBondMaturityDate2 = Date(17,February,2028);
    Date zerocpbondRedemption2 = bondCalendar.adjust(zeroCpnBondMaturityDate2,
                                                      Following);
    Leg zeroCpnBondLeg2 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zerocpbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> zeroCpnBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate2, zeroCpnBondStartDate2, zeroCpnBondLeg2));
    zeroCpnBond2->setPricingEngine(bondEngine);

    // specialized zerocpn bond
    ext::shared_ptr<Bond> zeroCpnSpecializedBond2(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                   Date(17,February,2028),
                   Following,
                   100.0, Date(17,February,1998)));
    zeroCpnSpecializedBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondTheoValue2 = zeroCpnBond2->cleanPrice();
    Real zeroCpnSpecializedBondTheoValue2 =
        zeroCpnSpecializedBond2->cleanPrice();

    Real error15 =
        std::fabs(zeroCpnBondTheoValue2 -zeroCpnSpecializedBondTheoValue2);
    if (error15>tolerance) {
        BOOST_FAIL("wrong clean price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zerocpn bond's clean price: "
                    << zeroCpnBondTheoValue2
                    << "\n  specialized zerocpn bond's clean price: "
                    << zeroCpnSpecializedBondTheoValue2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error15
                    << "\n  tolerance:             " << tolerance);
    }
    Real zeroCpnBondTheoDirty2 = zeroCpnBondTheoValue2+
                                 zeroCpnBond2->accruedAmount();

    Real zeroCpnSpecializedBondTheoDirty2 =
        zeroCpnSpecializedBondTheoValue2+
        zeroCpnSpecializedBond2->accruedAmount();

    Real error16 =
        std::fabs(zeroCpnBondTheoDirty2-zeroCpnSpecializedBondTheoDirty2);
    if (error16>tolerance) {
        BOOST_FAIL("wrong dirty price for zero coupon bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zerocpn bond's dirty price: "
                    << zeroCpnBondTheoDirty2
                    << "\n  specialized zerocpn bond's dirty price: "
                    << zeroCpnSpecializedBondTheoDirty2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error16
                    << "\n  tolerance:             " << tolerance);
    }
}


void AssetSwapTest::testSpecializedBondVsGenericBondUsingAsw() {

    BOOST_TEST_MESSAGE("Testing asset-swap prices and spreads for specialized"
                       " bond against equivalent generic bond...");

    using namespace asset_swap_test;

    CommonVars vars;

    Calendar bondCalendar = TARGET();
    Natural settlementDays = 3;
    Natural fixingDays = 2;
    bool payFixedRate = true;
    bool parAssetSwap = true;
    bool inArrears = false;

    // Fixed bond (Isin: DE0001135275 DBR 4 01/04/37)
    // maturity doesn't occur on a business day
    Date fixedBondStartDate1 = Date(4,January,2005);
    Date fixedBondMaturityDate1 = Date(4,January,2037);
    Schedule fixedBondSchedule1(fixedBondStartDate1,
                                fixedBondMaturityDate1,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg1 = FixedRateLeg(fixedBondSchedule1)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.04, ActualActual(ActualActual::ISDA));
    Date fixedbondRedemption1 = bondCalendar.adjust(fixedBondMaturityDate1,
                                                    Following);
    fixedBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> fixedBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate1, fixedBondStartDate1,
             fixedBondLeg1));
    ext::shared_ptr<PricingEngine> bondEngine(
                               new DiscountingBondEngine(vars.termStructure));
    ext::shared_ptr<PricingEngine> swapEngine(
                               new DiscountingSwapEngine(vars.termStructure));
    fixedBond1->setPricingEngine(bondEngine);

    // equivalent specialized fixed rate bond
    ext::shared_ptr<Bond> fixedSpecializedBond1(new
        FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule1,
                      std::vector<Rate>(1, 0.04),
                      ActualActual(ActualActual::ISDA), Following,
                      100.0, Date(4,January,2005) ));
    fixedSpecializedBond1->setPricingEngine(bondEngine);

    Real fixedBondPrice1 = fixedBond1->cleanPrice();
    Real fixedSpecializedBondPrice1 = fixedSpecializedBond1->cleanPrice();
    AssetSwap fixedBondAssetSwap1(payFixedRate,
                                  fixedBond1, fixedBondPrice1,
                                  vars.iborIndex, vars.nonnullspread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap1.setPricingEngine(swapEngine);
    AssetSwap fixedSpecializedBondAssetSwap1(payFixedRate,
                                             fixedSpecializedBond1,
                                             fixedSpecializedBondPrice1,
                                             vars.iborIndex,
                                             vars.nonnullspread,
                                             Schedule(),
                                             vars.iborIndex->dayCounter(),
                                             parAssetSwap);
    fixedSpecializedBondAssetSwap1.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice1 = fixedBondAssetSwap1.fairCleanPrice();
    Real fixedSpecializedBondAssetSwapPrice1 =
        fixedSpecializedBondAssetSwap1.fairCleanPrice();
    Real tolerance = 1.0e-13;
    Real error1 =
        std::fabs(fixedBondAssetSwapPrice1-fixedSpecializedBondAssetSwapPrice1);
    if (error1>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  fixed rate bond's  clean price: "
                    << fixedBondAssetSwapPrice1
                    << "\n  equivalent specialized bond's clean price: "
                    << fixedSpecializedBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error1
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real fixedBondMktPrice1= 91.832;
    AssetSwap fixedBondASW1(payFixedRate,
                            fixedBond1, fixedBondMktPrice1,
                            vars.iborIndex, vars.spread,
                            Schedule(),
                            vars.iborIndex->dayCounter(),
                            parAssetSwap);
    fixedBondASW1.setPricingEngine(swapEngine);
    AssetSwap fixedSpecializedBondASW1(payFixedRate,
                                       fixedSpecializedBond1,
                                       fixedBondMktPrice1,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    fixedSpecializedBondASW1.setPricingEngine(swapEngine);
    Real fixedBondASWSpread1 = fixedBondASW1.fairSpread();
    Real fixedSpecializedBondASWSpread1 = fixedSpecializedBondASW1.fairSpread();
    Real error2 = std::fabs(fixedBondASWSpread1-fixedSpecializedBondASWSpread1);
    if (error2>tolerance) {
        BOOST_FAIL("wrong asw spread  for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  fixed rate bond's  asw spread: "
                    << fixedBondASWSpread1
                    << "\n  equivalent specialized bond's asw spread: "
                    << fixedSpecializedBondASWSpread1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error2
                    << "\n  tolerance:             " << tolerance);
    }

     //Fixed bond (Isin: IT0006527060 IBRD 5 02/05/19)
     //maturity occurs on a business day

    Date fixedBondStartDate2 = Date(5,February,2005);
    Date fixedBondMaturityDate2 = Date(5,February,2019);
    Schedule fixedBondSchedule2(fixedBondStartDate2,
                                fixedBondMaturityDate2,
                                Period(Annual), bondCalendar,
                                Unadjusted, Unadjusted,
                                DateGeneration::Backward, false);
    Leg fixedBondLeg2 = FixedRateLeg(fixedBondSchedule2)
        .withNotionals(vars.faceAmount)
        .withCouponRates(0.05, Thirty360(Thirty360::BondBasis));
    Date fixedbondRedemption2 = bondCalendar.adjust(fixedBondMaturityDate2,
                                                    Following);
    fixedBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, fixedbondRedemption2)));

    // generic bond
    ext::shared_ptr<Bond> fixedBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             fixedBondMaturityDate2, fixedBondStartDate2, fixedBondLeg2));
    fixedBond2->setPricingEngine(bondEngine);

    // equivalent specialized fixed rate bond
    ext::shared_ptr<Bond> fixedSpecializedBond2(new
         FixedRateBond(settlementDays, vars.faceAmount, fixedBondSchedule2,
                      std::vector<Rate>(1, 0.05),
                      Thirty360(Thirty360::BondBasis), Following,
                      100.0, Date(5,February,2005)));
    fixedSpecializedBond2->setPricingEngine(bondEngine);

    Real fixedBondPrice2 = fixedBond2->cleanPrice();
    Real fixedSpecializedBondPrice2 = fixedSpecializedBond2->cleanPrice();
    AssetSwap fixedBondAssetSwap2(payFixedRate,
                                  fixedBond2, fixedBondPrice2,
                                  vars.iborIndex, vars.nonnullspread,
                                  Schedule(),
                                  vars.iborIndex->dayCounter(),
                                  parAssetSwap);
    fixedBondAssetSwap2.setPricingEngine(swapEngine);
    AssetSwap fixedSpecializedBondAssetSwap2(payFixedRate,
                                             fixedSpecializedBond2,
                                             fixedSpecializedBondPrice2,
                                             vars.iborIndex,
                                             vars.nonnullspread,
                                             Schedule(),
                                             vars.iborIndex->dayCounter(),
                                             parAssetSwap);
    fixedSpecializedBondAssetSwap2.setPricingEngine(swapEngine);
    Real fixedBondAssetSwapPrice2 = fixedBondAssetSwap2.fairCleanPrice();
    Real fixedSpecializedBondAssetSwapPrice2 =
        fixedSpecializedBondAssetSwap2.fairCleanPrice();

    Real error3 =
        std::fabs(fixedBondAssetSwapPrice2-fixedSpecializedBondAssetSwapPrice2);
    if (error3>tolerance) {
        BOOST_FAIL("wrong clean price for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  fixed rate bond's clean price: "
                    << fixedBondAssetSwapPrice2
                    << "\n  equivalent specialized  bond's clean price: "
                    << fixedSpecializedBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error3
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real fixedBondMktPrice2= 102.178;
    AssetSwap fixedBondASW2(payFixedRate,
                            fixedBond2, fixedBondMktPrice2,
                            vars.iborIndex, vars.spread,
                            Schedule(),
                            vars.iborIndex->dayCounter(),
                            parAssetSwap);
    fixedBondASW2.setPricingEngine(swapEngine);
    AssetSwap fixedSpecializedBondASW2(payFixedRate,
                                       fixedSpecializedBond2,
                                       fixedBondMktPrice2,
                                       vars.iborIndex, vars.spread,
                                       Schedule(),
                                       vars.iborIndex->dayCounter(),
                                       parAssetSwap);
    fixedSpecializedBondASW2.setPricingEngine(swapEngine);
    Real fixedBondASWSpread2 = fixedBondASW2.fairSpread();
    Real fixedSpecializedBondASWSpread2 = fixedSpecializedBondASW2.fairSpread();
    Real error4 = std::fabs(fixedBondASWSpread2-fixedSpecializedBondASWSpread2);
    if (error4>tolerance) {
        BOOST_FAIL("wrong asw spread for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  fixed rate bond's  asw spread: "
                    << fixedBondASWSpread2
                    << "\n  equivalent specialized bond's asw spread: "
                    << fixedSpecializedBondASWSpread2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error4
                    << "\n  tolerance:             " << tolerance);
    }


    //FRN bond (Isin: IT0003543847 ISPIM 0 09/29/13)
    //maturity doesn't occur on a business day
    Date floatingBondStartDate1 = Date(29,September,2003);
    Date floatingBondMaturityDate1 = Date(29,September,2013);
    Schedule floatingBondSchedule1(floatingBondStartDate1,
                                   floatingBondMaturityDate1,
                                   Period(Semiannual), bondCalendar,
                                   Unadjusted, Unadjusted,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg1 = IborLeg(floatingBondSchedule1, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withFixingDays(fixingDays)
        .withSpreads(0.0056)
        .inArrears(inArrears);
    Date floatingbondRedemption1 =
        bondCalendar.adjust(floatingBondMaturityDate1, Following);
    floatingBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> floatingBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate1, floatingBondStartDate1,
             floatingBondLeg1));
    floatingBond1->setPricingEngine(bondEngine);

    // equivalent specialized floater
    ext::shared_ptr<Bond> floatingSpecializedBond1(new
           FloatingRateBond(settlementDays, vars.faceAmount,
                            floatingBondSchedule1,
                            vars.iborIndex, Actual360(),
                            Following, fixingDays,
                            std::vector<Real>(1,1),
                            std::vector<Spread>(1,0.0056),
                            std::vector<Rate>(), std::vector<Rate>(),
                            inArrears,
                            100.0, Date(29,September,2003)));
    floatingSpecializedBond1->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond1->cashflows(), vars.pricer);
    setCouponPricer(floatingSpecializedBond1->cashflows(), vars.pricer);
    vars.iborIndex->addFixing(Date(27,March,2007), 0.0402);
    Real floatingBondPrice1 = floatingBond1->cleanPrice();
    Real floatingSpecializedBondPrice1= floatingSpecializedBond1->cleanPrice();
    AssetSwap floatingBondAssetSwap1(payFixedRate,
                                     floatingBond1, floatingBondPrice1,
                                     vars.iborIndex, vars.nonnullspread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap1.setPricingEngine(swapEngine);
    AssetSwap floatingSpecializedBondAssetSwap1(payFixedRate,
                                                floatingSpecializedBond1,
                                                floatingSpecializedBondPrice1,
                                                vars.iborIndex,
                                                vars.nonnullspread,
                                                Schedule(),
                                                vars.iborIndex->dayCounter(),
                                                parAssetSwap);
    floatingSpecializedBondAssetSwap1.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice1 = floatingBondAssetSwap1.fairCleanPrice();
    Real floatingSpecializedBondAssetSwapPrice1 =
        floatingSpecializedBondAssetSwap1.fairCleanPrice();

    Real error5 =
        std::fabs(floatingBondAssetSwapPrice1-floatingSpecializedBondAssetSwapPrice1);
    if (error5>tolerance) {
        BOOST_FAIL("wrong clean price for frnbond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic frn rate bond's clean price: "
                    << floatingBondAssetSwapPrice1
                    << "\n  equivalent specialized  bond's price: "
                    << floatingSpecializedBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error5
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real floatingBondMktPrice1= 101.33;
    AssetSwap floatingBondASW1(payFixedRate,
                               floatingBond1, floatingBondMktPrice1,
                               vars.iborIndex, vars.spread,
                               Schedule(),
                               vars.iborIndex->dayCounter(),
                               parAssetSwap);
    floatingBondASW1.setPricingEngine(swapEngine);
    AssetSwap floatingSpecializedBondASW1(payFixedRate,
                                          floatingSpecializedBond1,
                                          floatingBondMktPrice1,
                                          vars.iborIndex, vars.spread,
                                          Schedule(),
                                          vars.iborIndex->dayCounter(),
                                          parAssetSwap);
    floatingSpecializedBondASW1.setPricingEngine(swapEngine);
    Real floatingBondASWSpread1 = floatingBondASW1.fairSpread();
    Real floatingSpecializedBondASWSpread1 =
        floatingSpecializedBondASW1.fairSpread();
    Real error6 =
        std::fabs(floatingBondASWSpread1-floatingSpecializedBondASWSpread1);
    if (error6>tolerance) {
        BOOST_FAIL("wrong asw spread for fixed bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  frn rate bond's  asw spread: "
                    << floatingBondASWSpread1
                    << "\n  equivalent specialized bond's asw spread: "
                    << floatingSpecializedBondASWSpread1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error6
                    << "\n  tolerance:             " << tolerance);
    }
    //FRN bond (Isin: XS0090566539 COE 0 09/24/18)
    //maturity occurs on a business day
    Date floatingBondStartDate2 = Date(24,September,2004);
    Date floatingBondMaturityDate2 = Date(24,September,2018);
    Schedule floatingBondSchedule2(floatingBondStartDate2,
                                   floatingBondMaturityDate2,
                                   Period(Semiannual), bondCalendar,
                                   ModifiedFollowing, ModifiedFollowing,
                                   DateGeneration::Backward, false);
    Leg floatingBondLeg2 = IborLeg(floatingBondSchedule2, vars.iborIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Actual360())
        .withPaymentAdjustment(ModifiedFollowing)
        .withFixingDays(fixingDays)
        .withSpreads(0.0025)
        .inArrears(inArrears);
    Date floatingbondRedemption2 =
        bondCalendar.adjust(floatingBondMaturityDate2,
                            ModifiedFollowing);
    floatingBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, floatingbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> floatingBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             floatingBondMaturityDate2, floatingBondStartDate2,
             floatingBondLeg2));
    floatingBond2->setPricingEngine(bondEngine);

    // equivalent specialized floater
    ext::shared_ptr<Bond> floatingSpecializedBond2(new
        FloatingRateBond(settlementDays, vars.faceAmount,
                         floatingBondSchedule2,
                         vars.iborIndex, Actual360(),
                         ModifiedFollowing, fixingDays,
                         std::vector<Real>(1,1),
                         std::vector<Spread>(1,0.0025),
                         std::vector<Rate>(), std::vector<Rate>(),
                         inArrears,
                         100.0, Date(24,September,2004)));
    floatingSpecializedBond2->setPricingEngine(bondEngine);

    setCouponPricer(floatingBond2->cashflows(), vars.pricer);
    setCouponPricer(floatingSpecializedBond2->cashflows(), vars.pricer);

    vars.iborIndex->addFixing(Date(22,March,2007), 0.04013);

    Real floatingBondPrice2 = floatingBond2->cleanPrice();
    Real floatingSpecializedBondPrice2= floatingSpecializedBond2->cleanPrice();
    AssetSwap floatingBondAssetSwap2(payFixedRate,
                                     floatingBond2, floatingBondPrice2,
                                     vars.iborIndex, vars.nonnullspread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    floatingBondAssetSwap2.setPricingEngine(swapEngine);
    AssetSwap floatingSpecializedBondAssetSwap2(payFixedRate,
                                                floatingSpecializedBond2,
                                                floatingSpecializedBondPrice2,
                                                vars.iborIndex,
                                                vars.nonnullspread,
                                                Schedule(),
                                                vars.iborIndex->dayCounter(),
                                                parAssetSwap);
    floatingSpecializedBondAssetSwap2.setPricingEngine(swapEngine);
    Real floatingBondAssetSwapPrice2 = floatingBondAssetSwap2.fairCleanPrice();
    Real floatingSpecializedBondAssetSwapPrice2 =
        floatingSpecializedBondAssetSwap2.fairCleanPrice();
    Real error7 =
        std::fabs(floatingBondAssetSwapPrice2-floatingSpecializedBondAssetSwapPrice2);
    if (error7>tolerance) {
        BOOST_FAIL("wrong clean price for frnbond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic frn rate bond's clean price: "
                    << floatingBondAssetSwapPrice2
                    << "\n  equivalent specialized frn  bond's price: "
                    << floatingSpecializedBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error7
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real floatingBondMktPrice2 = 101.26;
    AssetSwap floatingBondASW2(payFixedRate,
                               floatingBond2, floatingBondMktPrice2,
                               vars.iborIndex, vars.spread,
                               Schedule(),
                               vars.iborIndex->dayCounter(),
                               parAssetSwap);
    floatingBondASW2.setPricingEngine(swapEngine);
    AssetSwap floatingSpecializedBondASW2(payFixedRate,
                                          floatingSpecializedBond2,
                                          floatingBondMktPrice2,
                                          vars.iborIndex, vars.spread,
                                          Schedule(),
                                          vars.iborIndex->dayCounter(),
                                          parAssetSwap);
    floatingSpecializedBondASW2.setPricingEngine(swapEngine);
    Real floatingBondASWSpread2 = floatingBondASW2.fairSpread();
    Real floatingSpecializedBondASWSpread2 =
        floatingSpecializedBondASW2.fairSpread();
    Real error8 =
        std::fabs(floatingBondASWSpread2-floatingSpecializedBondASWSpread2);
    if (error8>tolerance) {
        BOOST_FAIL("wrong asw spread for frn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  frn rate bond's  asw spread: "
                    << floatingBondASWSpread2
                    << "\n  equivalent specialized bond's asw spread: "
                    << floatingSpecializedBondASWSpread2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error8
                    << "\n  tolerance:             " << tolerance);
    }

    // CMS bond (Isin: XS0228052402 CRDIT 0 8/22/20)
    // maturity doesn't occur on a business day
    Date cmsBondStartDate1 = Date(22,August,2005);
    Date cmsBondMaturityDate1 = Date(22,August,2020);
    Schedule cmsBondSchedule1(cmsBondStartDate1,
                              cmsBondMaturityDate1,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg1 = CmsLeg(cmsBondSchedule1, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withCaps(0.055)
        .withFloors(0.025)
        .inArrears(inArrears);
    Date cmsbondRedemption1 = bondCalendar.adjust(cmsBondMaturityDate1,
                                                  Following);
    cmsBondLeg1.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption1)));
    // generic cms bond
    ext::shared_ptr<Bond> cmsBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate1, cmsBondStartDate1, cmsBondLeg1));
    cmsBond1->setPricingEngine(bondEngine);

    // equivalent specialized cms bond
    ext::shared_ptr<Bond> cmsSpecializedBond1(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule1,
                vars.swapIndex, Thirty360(Thirty360::BondBasis),
                Following, fixingDays,
                std::vector<Real>(1,1.0), std::vector<Spread>(1,0.0),
                std::vector<Rate>(1,0.055), std::vector<Rate>(1,0.025),
                inArrears,
                100.0, Date(22,August,2005)));
    cmsSpecializedBond1->setPricingEngine(bondEngine);


    setCouponPricer(cmsBond1->cashflows(), vars.cmspricer);
    setCouponPricer(cmsSpecializedBond1->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(18,August,2006), 0.04158);
    Real cmsBondPrice1 = cmsBond1->cleanPrice();
    Real cmsSpecializedBondPrice1 = cmsSpecializedBond1->cleanPrice();
    AssetSwap cmsBondAssetSwap1(payFixedRate,cmsBond1, cmsBondPrice1,
                                vars.iborIndex, vars.nonnullspread,
                                Schedule(),vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap1.setPricingEngine(swapEngine);
    AssetSwap cmsSpecializedBondAssetSwap1(payFixedRate,cmsSpecializedBond1,
                                           cmsSpecializedBondPrice1,
                                           vars.iborIndex,
                                           vars.nonnullspread,
                                           Schedule(),
                                           vars.iborIndex->dayCounter(),
                                           parAssetSwap);
    cmsSpecializedBondAssetSwap1.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice1 = cmsBondAssetSwap1.fairCleanPrice();
    Real cmsSpecializedBondAssetSwapPrice1 =
        cmsSpecializedBondAssetSwap1.fairCleanPrice();
    Real error9 =
        std::fabs(cmsBondAssetSwapPrice1-cmsSpecializedBondAssetSwapPrice1);
    if (error9>tolerance) {
        BOOST_FAIL("wrong clean price for cmsbond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic bond's clean price: "
                    << cmsBondAssetSwapPrice1
                    << "\n  equivalent specialized cms rate bond's price: "
                    << cmsSpecializedBondAssetSwapPrice1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error9
                    << "\n  tolerance:             " << tolerance);
    }
    Real cmsBondMktPrice1 = 87.02;// market executable price as of 4th sept 2007
    AssetSwap cmsBondASW1(payFixedRate,
                          cmsBond1, cmsBondMktPrice1,
                          vars.iborIndex, vars.spread,
                          Schedule(),
                          vars.iborIndex->dayCounter(),
                          parAssetSwap);
    cmsBondASW1.setPricingEngine(swapEngine);
    AssetSwap cmsSpecializedBondASW1(payFixedRate,
                                     cmsSpecializedBond1,
                                     cmsBondMktPrice1,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    cmsSpecializedBondASW1.setPricingEngine(swapEngine);
    Real cmsBondASWSpread1 = cmsBondASW1.fairSpread();
    Real cmsSpecializedBondASWSpread1 = cmsSpecializedBondASW1.fairSpread();
    Real error10 = std::fabs(cmsBondASWSpread1-cmsSpecializedBondASWSpread1);
    if (error10>tolerance) {
        BOOST_FAIL("wrong asw spread for cm bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic cms rate bond's  asw spread: "
                    << cmsBondASWSpread1
                    << "\n  equivalent specialized bond's asw spread: "
                    << cmsSpecializedBondASWSpread1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error10
                    << "\n  tolerance:             " << tolerance);
    }

      //CMS bond (Isin: XS0218766664 ISPIM 0 5/6/15)
      //maturity occurs on a business day
    Date cmsBondStartDate2 = Date(06,May,2005);
    Date cmsBondMaturityDate2 = Date(06,May,2015);
    Schedule cmsBondSchedule2(cmsBondStartDate2,
                              cmsBondMaturityDate2,
                              Period(Annual), bondCalendar,
                              Unadjusted, Unadjusted,
                              DateGeneration::Backward, false);
    Leg cmsBondLeg2 = CmsLeg(cmsBondSchedule2, vars.swapIndex)
        .withNotionals(vars.faceAmount)
        .withPaymentDayCounter(Thirty360(Thirty360::BondBasis))
        .withFixingDays(fixingDays)
        .withGearings(0.84)
        .inArrears(inArrears);
    Date cmsbondRedemption2 = bondCalendar.adjust(cmsBondMaturityDate2,
                                                  Following);
    cmsBondLeg2.push_back(ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, cmsbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> cmsBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             cmsBondMaturityDate2, cmsBondStartDate2, cmsBondLeg2));
    cmsBond2->setPricingEngine(bondEngine);

    // equivalent specialized cms bond
    ext::shared_ptr<Bond> cmsSpecializedBond2(new
        CmsRateBond(settlementDays, vars.faceAmount, cmsBondSchedule2,
                vars.swapIndex, Thirty360(Thirty360::BondBasis),
                Following, fixingDays,
                std::vector<Real>(1,0.84), std::vector<Spread>(1,0.0),
                std::vector<Rate>(), std::vector<Rate>(),
                inArrears,
                100.0, Date(06,May,2005)));
    cmsSpecializedBond2->setPricingEngine(bondEngine);

    setCouponPricer(cmsBond2->cashflows(), vars.cmspricer);
    setCouponPricer(cmsSpecializedBond2->cashflows(), vars.cmspricer);
    vars.swapIndex->addFixing(Date(04,May,2006), 0.04217);
    Real cmsBondPrice2 = cmsBond2->cleanPrice();
    Real cmsSpecializedBondPrice2 = cmsSpecializedBond2->cleanPrice();
    AssetSwap cmsBondAssetSwap2(payFixedRate,cmsBond2, cmsBondPrice2,
                                vars.iborIndex, vars.nonnullspread,
                                Schedule(),
                                vars.iborIndex->dayCounter(),
                                parAssetSwap);
    cmsBondAssetSwap2.setPricingEngine(swapEngine);
    AssetSwap cmsSpecializedBondAssetSwap2(payFixedRate,cmsSpecializedBond2,
                                           cmsSpecializedBondPrice2,
                                           vars.iborIndex,
                                           vars.nonnullspread,
                                           Schedule(),
                                           vars.iborIndex->dayCounter(),
                                           parAssetSwap);
    cmsSpecializedBondAssetSwap2.setPricingEngine(swapEngine);
    Real cmsBondAssetSwapPrice2 = cmsBondAssetSwap2.fairCleanPrice();
    Real cmsSpecializedBondAssetSwapPrice2 =
        cmsSpecializedBondAssetSwap2.fairCleanPrice();
    Real error11 =
        std::fabs(cmsBondAssetSwapPrice2-cmsSpecializedBondAssetSwapPrice2);
    if (error11>tolerance) {
        BOOST_FAIL("wrong clean price for cmsbond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic  bond's clean price: "
                    << cmsBondAssetSwapPrice2
                    << "\n  equivalent specialized cms rate bond's price: "
                    << cmsSpecializedBondAssetSwapPrice2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error11
                    << "\n  tolerance:             " << tolerance);
    }
    Real cmsBondMktPrice2 = 94.35;// market executable price as of 4th sept 2007
    AssetSwap cmsBondASW2(payFixedRate,
                          cmsBond2, cmsBondMktPrice2,
                          vars.iborIndex, vars.spread,
                          Schedule(),
                          vars.iborIndex->dayCounter(),
                          parAssetSwap);
    cmsBondASW2.setPricingEngine(swapEngine);
    AssetSwap cmsSpecializedBondASW2(payFixedRate,
                                     cmsSpecializedBond2,
                                     cmsBondMktPrice2,
                                     vars.iborIndex, vars.spread,
                                     Schedule(),
                                     vars.iborIndex->dayCounter(),
                                     parAssetSwap);
    cmsSpecializedBondASW2.setPricingEngine(swapEngine);
    Real cmsBondASWSpread2 = cmsBondASW2.fairSpread();
    Real cmsSpecializedBondASWSpread2 = cmsSpecializedBondASW2.fairSpread();
    Real error12 = std::fabs(cmsBondASWSpread2-cmsSpecializedBondASWSpread2);
    if (error12>tolerance) {
        BOOST_FAIL("wrong asw spread for cm bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic cms rate bond's  asw spread: "
                    << cmsBondASWSpread2
                    << "\n  equivalent specialized bond's asw spread: "
                    << cmsSpecializedBondASWSpread2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error12
                    << "\n  tolerance:             " << tolerance);
    }


   //  Zero-Coupon bond (Isin: DE0004771662 IBRD 0 12/20/15)
   //  maturity doesn't occur on a business day
    Date zeroCpnBondStartDate1 = Date(19,December,1985);
    Date zeroCpnBondMaturityDate1 = Date(20,December,2015);
    Date zeroCpnBondRedemption1 = bondCalendar.adjust(zeroCpnBondMaturityDate1,
                                                      Following);
    Leg zeroCpnBondLeg1 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zeroCpnBondRedemption1)));
    // generic bond
    ext::shared_ptr<Bond> zeroCpnBond1(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate1, zeroCpnBondStartDate1, zeroCpnBondLeg1));
    zeroCpnBond1->setPricingEngine(bondEngine);

    // specialized zerocpn bond
    ext::shared_ptr<Bond> zeroCpnSpecializedBond1(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                  Date(20,December,2015),
                  Following,
                  100.0, Date(19,December,1985)));
    zeroCpnSpecializedBond1->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice1 = zeroCpnBond1->cleanPrice();
    Real zeroCpnSpecializedBondPrice1 = zeroCpnSpecializedBond1->cleanPrice();
    AssetSwap zeroCpnBondAssetSwap1(payFixedRate,zeroCpnBond1,
                                    zeroCpnBondPrice1,
                                    vars.iborIndex, vars.nonnullspread,
                                    Schedule(),
                                    vars.iborIndex->dayCounter(),
                                    parAssetSwap);
    zeroCpnBondAssetSwap1.setPricingEngine(swapEngine);
    AssetSwap zeroCpnSpecializedBondAssetSwap1(payFixedRate,
                                               zeroCpnSpecializedBond1,
                                               zeroCpnSpecializedBondPrice1,
                                               vars.iborIndex,
                                               vars.nonnullspread,
                                               Schedule(),
                                               vars.iborIndex->dayCounter(),
                                               parAssetSwap);
    zeroCpnSpecializedBondAssetSwap1.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice1 = zeroCpnBondAssetSwap1.fairCleanPrice();
    Real zeroCpnSpecializedBondAssetSwapPrice1 =
        zeroCpnSpecializedBondAssetSwap1.fairCleanPrice();
    Real error13 =
        std::fabs(zeroCpnBondAssetSwapPrice1-zeroCpnSpecializedBondAssetSwapPrice1);
    if (error13>tolerance) {
        BOOST_FAIL("wrong clean price for zerocpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zero cpn bond's clean price: "
                    << zeroCpnBondAssetSwapPrice1
                    << "\n  specialized equivalent bond's price: "
                    << zeroCpnSpecializedBondAssetSwapPrice1
                    << "\n  error:                 " << error13
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real zeroCpnBondMktPrice1 = 72.277;
    AssetSwap zeroCpnBondASW1(payFixedRate,
                              zeroCpnBond1,zeroCpnBondMktPrice1,
                              vars.iborIndex, vars.spread,
                              Schedule(),
                              vars.iborIndex->dayCounter(),
                              parAssetSwap);
    zeroCpnBondASW1.setPricingEngine(swapEngine);
    AssetSwap zeroCpnSpecializedBondASW1(payFixedRate,
                                         zeroCpnSpecializedBond1,
                                         zeroCpnBondMktPrice1,
                                         vars.iborIndex, vars.spread,
                                         Schedule(),
                                         vars.iborIndex->dayCounter(),
                                         parAssetSwap);
    zeroCpnSpecializedBondASW1.setPricingEngine(swapEngine);
    Real zeroCpnBondASWSpread1 = zeroCpnBondASW1.fairSpread();
    Real zeroCpnSpecializedBondASWSpread1 =
        zeroCpnSpecializedBondASW1.fairSpread();
    Real error14 =
        std::fabs(zeroCpnBondASWSpread1-zeroCpnSpecializedBondASWSpread1);
    if (error14>tolerance) {
        BOOST_FAIL("wrong asw spread for zeroCpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zeroCpn bond's  asw spread: "
                    << zeroCpnBondASWSpread1
                    << "\n  equivalent specialized bond's asw spread: "
                    << zeroCpnSpecializedBondASWSpread1
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error14
                    << "\n  tolerance:             " << tolerance);
    }


   //  Zero Coupon bond (Isin: IT0001200390 ISPIM 0 02/17/28)
   //  maturity doesn't occur on a business day
    Date zeroCpnBondStartDate2 = Date(17,February,1998);
    Date zeroCpnBondMaturityDate2 = Date(17,February,2028);
    Date zerocpbondRedemption2 = bondCalendar.adjust(zeroCpnBondMaturityDate2,
                                                      Following);
    Leg zeroCpnBondLeg2 = Leg(1, ext::shared_ptr<CashFlow>(new
        SimpleCashFlow(100.0, zerocpbondRedemption2)));
    // generic bond
    ext::shared_ptr<Bond> zeroCpnBond2(new
        Bond(settlementDays, bondCalendar, vars.faceAmount,
             zeroCpnBondMaturityDate2, zeroCpnBondStartDate2, zeroCpnBondLeg2));
    zeroCpnBond2->setPricingEngine(bondEngine);

    // specialized zerocpn bond
    ext::shared_ptr<Bond> zeroCpnSpecializedBond2(new
        ZeroCouponBond(settlementDays, bondCalendar, vars.faceAmount,
                   Date(17,February,2028),
                   Following,
                   100.0, Date(17,February,1998)));
    zeroCpnSpecializedBond2->setPricingEngine(bondEngine);

    Real zeroCpnBondPrice2 = zeroCpnBond2->cleanPrice();
    Real zeroCpnSpecializedBondPrice2 = zeroCpnSpecializedBond2->cleanPrice();

    AssetSwap zeroCpnBondAssetSwap2(payFixedRate,zeroCpnBond2,
                                    zeroCpnBondPrice2,
                                    vars.iborIndex, vars.nonnullspread,
                                    Schedule(),
                                    vars.iborIndex->dayCounter(),
                                    parAssetSwap);
    zeroCpnBondAssetSwap2.setPricingEngine(swapEngine);
    AssetSwap zeroCpnSpecializedBondAssetSwap2(payFixedRate,
                                               zeroCpnSpecializedBond2,
                                               zeroCpnSpecializedBondPrice2,
                                               vars.iborIndex,
                                               vars.nonnullspread,
                                               Schedule(),
                                               vars.iborIndex->dayCounter(),
                                               parAssetSwap);
    zeroCpnSpecializedBondAssetSwap2.setPricingEngine(swapEngine);
    Real zeroCpnBondAssetSwapPrice2 = zeroCpnBondAssetSwap2.fairCleanPrice();
    Real zeroCpnSpecializedBondAssetSwapPrice2 =
                               zeroCpnSpecializedBondAssetSwap2.fairCleanPrice();
    Real error15 = std::fabs(zeroCpnBondAssetSwapPrice2
                             -zeroCpnSpecializedBondAssetSwapPrice2);
    if (error15>tolerance) {
        BOOST_FAIL("wrong clean price for zerocpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zero cpn bond's clean price: "
                    << zeroCpnBondAssetSwapPrice2
                    << "\n  equivalent specialized bond's price: "
                    << zeroCpnSpecializedBondAssetSwapPrice2
                    << "\n  error:                 " << error15
                    << "\n  tolerance:             " << tolerance);
    }
    // market executable price as of 4th sept 2007
    Real zeroCpnBondMktPrice2 = 72.277;
    AssetSwap zeroCpnBondASW2(payFixedRate,
                              zeroCpnBond2,zeroCpnBondMktPrice2,
                              vars.iborIndex, vars.spread,
                              Schedule(),
                              vars.iborIndex->dayCounter(),
                              parAssetSwap);
    zeroCpnBondASW2.setPricingEngine(swapEngine);
    AssetSwap zeroCpnSpecializedBondASW2(payFixedRate,
                                         zeroCpnSpecializedBond2,
                                         zeroCpnBondMktPrice2,
                                         vars.iborIndex, vars.spread,
                                         Schedule(),
                                         vars.iborIndex->dayCounter(),
                                         parAssetSwap);
    zeroCpnSpecializedBondASW2.setPricingEngine(swapEngine);
    Real zeroCpnBondASWSpread2 = zeroCpnBondASW2.fairSpread();
    Real zeroCpnSpecializedBondASWSpread2 =
        zeroCpnSpecializedBondASW2.fairSpread();
    Real error16 =
        std::fabs(zeroCpnBondASWSpread2-zeroCpnSpecializedBondASWSpread2);
    if (error16>tolerance) {
        BOOST_FAIL("wrong asw spread for zeroCpn bond:"
                    << std::fixed << std::setprecision(4)
                    << "\n  generic zeroCpn bond's  asw spread: "
                    << zeroCpnBondASWSpread2
                    << "\n  equivalent specialized bond's asw spread: "
                    << zeroCpnSpecializedBondASWSpread2
                    << std::scientific << std::setprecision(2)
                    << "\n  error:                 " << error16
                    << "\n  tolerance:             " << tolerance);
    }
}


test_suite* AssetSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("AssetSwap tests");
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testConsistency));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testImpliedValue));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testMarketASWSpread));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testZSpread));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testGenericBondImplied));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testMASWWithGenericBond));
    suite->add(QUANTLIB_TEST_CASE(&AssetSwapTest::testZSpreadWithGenericBond));
    suite->add(QUANTLIB_TEST_CASE(
                           &AssetSwapTest::testSpecializedBondVsGenericBond));
    suite->add(QUANTLIB_TEST_CASE(
                   &AssetSwapTest::testSpecializedBondVsGenericBondUsingAsw));

    return suite;
}
