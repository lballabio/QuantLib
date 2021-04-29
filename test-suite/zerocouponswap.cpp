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
            finalPayment = 1.2e6;

            euribor = ext::shared_ptr<IborIndex>(new Euribor6M(euriborHandle));
            euribor->addFixing(Date(10, February, 2021), 0.0085);

            today = calendar.adjust(Date(15, March, 2021));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            euriborHandle.linkTo(flatRate(settlement, 0.007, dayCount));
            discountEngine =
                ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(euriborHandle));
        }

        ext::shared_ptr<CashFlow>
        createSubPeriodsCoupon(const Date& start,
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
                                                     Real baseNominal,
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
                                                     Real finalPayment,
                                                     RateAveraging::Type averaging) {
            return createZCSwap(type, start, end, baseNominal, finalPayment, averaging);
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
            auto swap = ext::make_shared<ZeroCouponSwap>(ZeroCouponSwap::Receiver, baseNominal, 
                                                         start, end, fixedRate, dayCount, euribor,
                                                         calendar, businessConvention, paymentDelay);
            swap->setPricingEngine(discountEngine);
            return swap;
        }
    };

    std::string printType(ZeroCouponSwap::Type type) {
        return type == ZeroCouponSwap::Receiver ? "receiver" : "payer";
    }

    std::string printAveraging(RateAveraging::Type type) {
        return type == RateAveraging::Compound ? "compound" : "simple";
    }

    void checkReplicationOfZeroCouponSwapNPV(const Date& start,
                                             const Date& end,
                                             ZeroCouponSwap::Type type = ZeroCouponSwap::Receiver,
                                             RateAveraging::Type averaging = RateAveraging::Compound) {
        CommonVars vars;
        const Real tolerance = 1.0e-9;

        auto zcSwap = vars.createZCSwap(type, start, end, averaging);

        Real actualNPV = zcSwap->NPV();
        Real actualFixedLegNPV = zcSwap->fixedLegNPV();
        Real actualFloatLegNPV = zcSwap->floatingLegNPV();

        Date paymentDate =
            vars.calendar.advance(end, vars.paymentDelay * Days, vars.businessConvention);
        Real discountAtPayment =
            paymentDate < vars.settlement ? 0.0 : vars.euriborHandle->discount(paymentDate);
        Real expectedFixedLegNPV = -type * discountAtPayment * vars.finalPayment;

        auto subPeriodCpn = vars.createSubPeriodsCoupon(start, end, averaging);
        Real expectedFloatLegNPV =
            paymentDate < vars.settlement ? 0.0 : type * discountAtPayment * subPeriodCpn->amount();

        Real expectedNPV = expectedFloatLegNPV + expectedFixedLegNPV;

        if ((std::fabs(actualNPV - expectedNPV) > tolerance) ||
            (std::fabs(actualFixedLegNPV - expectedFixedLegNPV) > tolerance) ||
            (std::fabs(actualFloatLegNPV - expectedFloatLegNPV) > tolerance))
            BOOST_ERROR("unable to replicate NPVs of zero coupon swap and it's legs\n"
                        << "    actual NPV:    " << actualNPV << "\n"
                        << "    expected NPV:    " << expectedNPV << "\n"
                        << "    actual fixed leg NPV:    " << actualFixedLegNPV << "\n"
                        << "    expected fixed leg NPV:    " << expectedFixedLegNPV << "\n"
                        << "    actual float leg NPV:    " << actualFloatLegNPV << "\n"
                        << "    expected float leg NPV:    " << expectedFloatLegNPV << "\n"
                        << "    start:    " << start << "\n"
                        << "    end:    " << end << "\n"
                        << "    type:    " << printType(type) << "\n"
                        << "    averaging:    " << printAveraging(averaging) << "\n");
    }

    void checkFairFixedPayment(const Date& start,
                               const Date& end,
                               ZeroCouponSwap::Type type,
                               RateAveraging::Type averaging) {
        CommonVars vars;
        const Real tolerance = 1.0e-9;

        auto zcSwap = vars.createZCSwap(type, start, end, averaging);
        Real fairFixedPayment = zcSwap->fairFixedPayment();
        auto parZCSwap = vars.createZCSwap(type, start, end, fairFixedPayment, averaging);
        Real parZCSwapNPV = parZCSwap->NPV();

        if ((std::fabs(parZCSwapNPV) > tolerance))
            BOOST_ERROR("unable to replicate fair fixed payment\n"
                        << "    actual NPV:    " << parZCSwapNPV << "\n"
                        << "    expected NPV:    0.0\n"
                        << "    fair fixed payment:    " << fairFixedPayment << "\n"
                        << "    start:    " << start << "\n"
                        << "    end:    " << end << "\n"
                        << "    type:    " << printType(type) << "\n"
                        << "    averaging:    " << printAveraging(averaging) << "\n");
    }
}

void ZeroCouponSwapTest::testInstrumentValuation() {
    BOOST_TEST_MESSAGE("Testing zero coupon swap valuation...");
    
    using namespace zerocouponswap_test;

    // Ongoing instrument
    checkReplicationOfZeroCouponSwapNPV(Date(12, February, 2021), Date(12, February, 2041),
                                        ZeroCouponSwap::Receiver, RateAveraging::Compound);
    // Forward starting instrument
    checkReplicationOfZeroCouponSwapNPV(Date(15, April, 2021), Date(12, February, 2041), 
                                        ZeroCouponSwap::Payer, RateAveraging::Simple);

    // Expired instrument
    checkReplicationOfZeroCouponSwapNPV(Date(12, February, 2000), Date(12, February, 2020));
}

void ZeroCouponSwapTest::testFairFixedPayment() {
    BOOST_TEST_MESSAGE("Testing fair fixed payment...");
    
    using namespace zerocouponswap_test;

    // Ongoing instrument
    checkFairFixedPayment(Date(12, February, 2021), Date(12, February, 2041),
                          ZeroCouponSwap::Receiver, RateAveraging::Compound);

    // Spot starting instrument
    checkFairFixedPayment(Date(17, March, 2021), Date(12, February, 2041), 
                          ZeroCouponSwap::Payer, RateAveraging::Simple);
}

void ZeroCouponSwapTest::testFixedPaymentFromRate() {
    BOOST_TEST_MESSAGE("Testing fixed payment calculation from rate...");

    using namespace zerocouponswap_test;
    
    CommonVars vars;
    const Real tolerance = 1.0e-9;
    const Rate fixedRate = 0.01;

    Date start(12, February, 2021);
    Date end(12, February, 2041);

    auto zcSwap = vars.createZCSwap(start, end, fixedRate);
    Real actualFxdPmt = zcSwap->fixedPayment();

    Time T = vars.dayCount.yearFraction(start, end);
    Real expectedFxdPmt = zcSwap->baseNominal() * (std::pow(1.0 + fixedRate, T) - 1.0);

    if ((std::fabs(actualFxdPmt - expectedFxdPmt) > tolerance))
        BOOST_ERROR("unable to replicate fixed payment from rate\n"
                    << "    actual fixed payment:    " << actualFxdPmt << "\n"
                    << "    expected fixed payment:    " << expectedFxdPmt << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void ZeroCouponSwapTest::testArgumentsValidation() {
    BOOST_TEST_MESSAGE("Testing arguments validation...");

    using namespace zerocouponswap_test;

    CommonVars vars;

    Date start(12, February, 2021);
    Date end(12, February, 2041);

    // Negative base nominal
    BOOST_CHECK_THROW(vars.createZCSwap(ZeroCouponSwap::Payer, start, end, -1.0e6, 1.0e6,
                                        RateAveraging::Compound),
                      Error);

    // Start date after end date
    BOOST_CHECK_THROW(vars.createZCSwap(end, start, 0.01), Error);
}

void ZeroCouponSwapTest::testExpectedCashFlowsInLegs() {
    BOOST_TEST_MESSAGE("Testing expected cash flows in legs...");

    using namespace zerocouponswap_test;

    CommonVars vars;
    const Real tolerance = 1.0e-9;

    Date start(12, February, 2021);
    Date end(12, February, 2041);

    auto zcSwap = vars.createZCSwap(start, end, 0.01);
    auto fixedCashFlow = zcSwap->fixedLeg()[0];
    auto floatingCashFlow = zcSwap->floatingLeg()[0];

    Date paymentDate =
        vars.calendar.advance(end, vars.paymentDelay * Days, vars.businessConvention);
    auto subPeriodCpn = vars.createSubPeriodsCoupon(start, end);


    if ((std::fabs(fixedCashFlow->amount() - zcSwap->fixedPayment()) > tolerance) ||
        (fixedCashFlow->date() != paymentDate))
        BOOST_ERROR("unable to replicate fixed leg\n"
                    << "    actual amount:    " << fixedCashFlow->amount() << "\n"
                    << "    expected amount:    " << zcSwap->fixedPayment() << "\n"
                    << "    actual payment date:    " << fixedCashFlow->date() << "\n"
                    << "    expected payment date:    " << paymentDate << "\n");

    if ((std::fabs(floatingCashFlow->amount() - subPeriodCpn->amount()) > tolerance) ||
        (floatingCashFlow->date() != paymentDate))
        BOOST_ERROR("unable to replicate floating leg\n"
                    << "    actual amount:    " << floatingCashFlow->amount() << "\n"
                    << "    expected amount:    " << subPeriodCpn->amount() << "\n"
                    << "    actual payment date:    " << floatingCashFlow->date() << "\n"
                    << "    expected payment date:    " << paymentDate << "\n");
}

test_suite* ZeroCouponSwapTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Zero coupon swap tests");

    suite->add(QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::testInstrumentValuation));
    suite->add(QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::testFairFixedPayment));
    suite->add(QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::testFixedPaymentFromRate));
    suite->add(QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::testArgumentsValidation));
    suite->add(QUANTLIB_TEST_CASE(&ZeroCouponSwapTest::testExpectedCashFlowsInLegs));

    return suite;
}
