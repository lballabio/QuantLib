/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2021 Marcin Rybacki

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

#include "zerocouponswap.hpp"
#include "utilities.hpp"
#include <ql/instruments/zerocouponswap.hpp>
#include <ql/cashflows/subperiodcoupon.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace zerocouponswap_test {

    struct CommonVars {

        Date today, settlement;
        Calendar calendar;
        Natural settlementDays, paymentDelay;
        DayCounter dayCount;
        BusinessDayConvention businessConvention;
        Real baseNominal, finalPayment;

        ext::shared_ptr<IborIndex> euribor;
        RelinkableHandle<YieldTermStructure> euriborHandle;
        ext::shared_ptr<PricingEngine> discountEngine;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            settlementDays = 2;
            paymentDelay = 1;
            calendar = TARGET();
            dayCount = Actual365Fixed();
            businessConvention = ModifiedFollowing;
            baseNominal = 1.0e6;
            finalPayment = 1.5e6;

            euribor = ext::shared_ptr<IborIndex>(new Euribor6M(euriborHandle));
            euribor->addFixing(Date(10, February, 2021), 0.0085);

            today = calendar.adjust(Date(15, March, 2021));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            euriborHandle.linkTo(flatRate(settlement, 0.007, dayCount));
            discountEngine = ext::shared_ptr<PricingEngine>(
                new DiscountingSwapEngine(euriborHandle));
        }

        ext::shared_ptr<CashFlow> createSubPeriodsCoupon(const Date& start,
                                                         const Date& end,
                                                         RateAveraging::Type averaging = RateAveraging::Compound) {
            Date paymentDate = calendar.advance(end, paymentDelay * Days, businessConvention);
            ext::shared_ptr<FloatingRateCoupon> cpn(new SubPeriodsCoupon(
                paymentDate, baseNominal, start, end, settlementDays, euribor));
            bool useCompoundedRate = (averaging == RateAveraging::Compound);
            if (useCompoundedRate)
                cpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new CompoundingRatePricer()));
            else
                cpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new AveragingRatePricer()));
            return cpn;
        }

        ext::shared_ptr<ZeroCouponSwap> createZCSwap(ZeroCouponSwap::Type type,
                                                     const Date& start,
                                                     const Date& end,
                                                     Real finalPayment,
                                                     RateAveraging::Type averaging) {
            auto swap = ext::make_shared<ZeroCouponSwap>(type, baseNominal, start, end, finalPayment,
                                                         euribor, calendar, businessConvention,
                                                         paymentDelay, averaging);
            swap->setPricingEngine(discountEngine);
            return swap;
        }

        ext::shared_ptr<ZeroCouponSwap> createZCSwap(ZeroCouponSwap::Type type,
                                                     const Date& start,
                                                     const Date& end,
                                                     RateAveraging::Type averaging) {
            return createZCSwap(type, start, end, finalPayment, averaging);
        }

        ext::shared_ptr<ZeroCouponSwap> createZCSwap(const Date& start,
                                                     const Date& end,
                                                     Rate fixedRate) {
            auto swap =  ext::make_shared<ZeroCouponSwap>(ZeroCouponSwap::Receiver, baseNominal, start, end,
                                                          fixedRate, dayCount, euribor, calendar, 
                                                          businessConvention, paymentDelay, RateAveraging::Compound);
            swap->setPricingEngine(discountEngine);
            return swap;
        }
    };
}

void testReplicationOfZeroCouponSwapNpv(ZeroCouponSwap::Type type,
                                        const Date& start,
                                        const Date& end,
                                        RateAveraging::Type averaging) {
    using namespace zerocouponswap_test;
    CommonVars vars;
    const Real tolerance = 1.0e-9;

    auto zcSwap = vars.createZCSwap(type, start, end, averaging);

    Real actualNpv = zcSwap->NPV();
    Real actualFixedLegNpv = zcSwap->fixedLegNPV();
    Real actualFloatLegNpv = zcSwap->floatingLegNPV();
    
    Date paymentDate = vars.calendar.advance(
        end, vars.paymentDelay * Days, vars.businessConvention);
    Real discountAtPayment = vars.euriborHandle->discount(paymentDate);
    Real expectedFixedLegNpv = discountAtPayment * vars.finalPayment;
    
    auto subPeriodCpn = vars.createSubPeriodsCoupon(start, end, averaging);
    Real expectedFloatLegNpv = discountAtPayment * subPeriodCpn->amount();

    Real expectedNpv = type * (expectedFloatLegNpv - expectedFixedLegNpv);

    if ((std::fabs(actualNpv - expectedNpv) > tolerance) || 
        (std::fabs(actualFixedLegNpv - expectedFixedLegNpv) > tolerance) ||
        (std::fabs(actualFloatLegNpv - expectedFloatLegNpv) > tolerance))
        BOOST_ERROR("unable to replicate NPVs of zero coupon swap and it's legs\n"
                    << std::setprecision(2) << "    actual NPV:    " << actualNpv << "\n"
                    << std::setprecision(2) << "    expected NPV:    " << expectedNpv << "\n"
                    << std::setprecision(2) << "    actual fixed leg NPV:    " << actualFixedLegNpv << "\n"
                    << std::setprecision(2) << "    expected fixed leg NPV:    " << expectedFixedLegNpv << "\n"
                    << std::setprecision(2) << "    actual float leg NPV:    " << actualFloatLegNpv << "\n"
                    << std::setprecision(2) << "    expected float leg NPV:    " << expectedFloatLegNpv << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n"
                    << "    averaging:    " << averaging << "\n");
}

void ZeroCouponSwapTest::test() {
    BOOST_TEST_MESSAGE("Testing ongoing zero coupon swap NPVs...");

    Date start(12, February, 2021);
    Date end(12, February, 2041);

    testReplicationOfZeroCouponSwapNpv(ZeroCouponSwap::Receiver, start, end,
                                       RateAveraging::Compound);
    testReplicationOfZeroCouponSwapNpv(ZeroCouponSwap::Payer, start, end,
                                       RateAveraging::Simple);
}

test_suite* ZeroCouponSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Zero coupon swap tests");

    suite->add(
        QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::test));

    return suite;
}