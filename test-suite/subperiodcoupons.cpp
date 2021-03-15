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

#include "subperiodcoupons.hpp"
#include "utilities.hpp"
#include <ql/experimental/coupons/subperiodcoupons.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/time/calendars/target.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace subperiodcoupons_test {

    struct CommonVars {

        Date today, settlement;
        Calendar calendar;
        Natural settlementDays;
        DayCounter dayCount;
        BusinessDayConvention businessConvention;

        ext::shared_ptr<IborIndex> euribor;
        RelinkableHandle<YieldTermStructure> euriborHandle;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            settlementDays = 2;
            calendar = TARGET();
            dayCount = Actual365Fixed();
            businessConvention = ModifiedFollowing;

            euribor = ext::shared_ptr<IborIndex>(new Euribor6M(euriborHandle));
            euribor->addFixing(Date(10, February, 2021), 0.0085);

            today = calendar.adjust(Date(15, March, 2021));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            euriborHandle.linkTo(flatRate(settlement, 0.007, dayCount));
        }

        Leg createIborLeg(const Date& start, const Date& end, Spread spread) {
            Schedule sch = MakeSchedule()
                               .from(start)
                               .to(end)
                               .withTenor(euribor->tenor())
                               .withCalendar(euribor->fixingCalendar())
                               .withConvention(euribor->businessDayConvention())
                               .backwards()
                               .endOfMonth(euribor->endOfMonth());
            return IborLeg(sch, euribor)
                .withNotionals(1.0)
                .withSpreads(spread)
                .withExCouponPeriod(2 * Days, calendar, businessConvention)
                .withPaymentLag(1)
                .withFixingDays(settlementDays);
        }

        ext::shared_ptr<CashFlow>
        createSubPeriodsCoupon(const Date& start, const Date& end, Spread rateSpread, bool useCompoundedRate) {
            Date paymentDate = calendar.advance(end, 1 * Days, businessConvention);
            Date exCouponDate = calendar.advance(paymentDate, -2 * Days, businessConvention);
            ext::shared_ptr<FloatingRateCoupon> cpn(
                new SubPeriodsCoupon(paymentDate, 1.0, start, end, settlementDays, euribor, 1.0,
                                     0.0, rateSpread, Date(), Date(), DayCounter(), exCouponDate));
            if (useCompoundedRate)
                cpn->setPricer(ext::shared_ptr<FloatingRateCouponPricer>(new CompoundingRatePricer()));
            else
                cpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new AveragingRatePricer()));
            return cpn;
        }
    };

    Real sumIborLegPayments(const Leg& leg)
    {
        Real payments = 0.0;
        std::for_each(leg.begin(), leg.end(), [&payments](const ext::shared_ptr<CashFlow>& cf) {
            payments += cf->amount();
        });
        return payments;
    }

    Real compoundedIborLegPayment(const Leg& leg) {
        Real compound = 1.0;
        std::for_each(leg.begin(), leg.end(), [&compound](const ext::shared_ptr<CashFlow>& cf) {
            auto cpn = ext::dynamic_pointer_cast<IborCoupon>(cf);
            Real yearFraction = cpn->accrualPeriod();
            Rate fixing = cpn->indexFixing();
            compound *= (1.0 + yearFraction * (fixing + cpn->spread()));
        });
        return (compound - 1.0);
    }

    Real averagedIborLegPayment(const Leg& leg) {
        Real acc = 1.0;
        std::for_each(leg.begin(), leg.end(), [&acc](const ext::shared_ptr<CashFlow>& cf) {
            auto cpn = ext::dynamic_pointer_cast<IborCoupon>(cf);
            Real yearFraction = cpn->accrualPeriod();
            Rate fixing = cpn->indexFixing();
            acc += yearFraction * (fixing + cpn->spread());
        });
        return acc;
    }
}

void testSinglePeriodCouponReplication(const Date& start,
                                       const Date& end,
                                       Spread rateSpread,
                                       bool useCompoundedRate) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Leg iborLeg = vars.createIborLeg(start, end, rateSpread);
    ext::shared_ptr<CashFlow> subPeriodCpn =
        vars.createSubPeriodsCoupon(start, end, rateSpread, useCompoundedRate);

    Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();
    Real expectedPayment = sumIborLegPayments(iborLeg);

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate single period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void testMultipleCompoundedSubPeriodsCouponReplication(const Date& start,
                                                       const Date& end,
                                                       Spread rateSpread) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Leg iborLeg = vars.createIborLeg(start, end, rateSpread);
    
    bool useCompoundedRate = true;
    ext::shared_ptr<CashFlow> subPeriodCpn =
        vars.createSubPeriodsCoupon(start, end, rateSpread, useCompoundedRate);

    Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();
    Real expectedPayment = compoundedIborLegPayment(iborLeg);

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate compounded multiple sub-period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void testMultipleAveragedSubPeriodsCouponReplication(const Date& start,
                                                     const Date& end,
                                                     Spread spread) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Leg iborLeg = vars.createIborLeg(start, end, spread);
    
    bool useCompoundedRate = false;
    ext::shared_ptr<CashFlow> subPeriodCpn =
        vars.createSubPeriodsCoupon(start, end, spread, useCompoundedRate);

    Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();
    Real expectedPayment = averagedIborLegPayment(iborLeg);

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate averaged multiple sub-period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void SubPeriodsCouponTest::testRegularSinglePeriodForwardStartingCoupon() {
    BOOST_TEST_MESSAGE("Testing regular single period forward starting coupon...");

    Date start(15, April, 2021);
    Date end(15, October, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, true);
    testSinglePeriodCouponReplication(start, end, spread, false);
}

void SubPeriodsCouponTest::testRegularSinglePeriodCouponAfterFixing() {
    BOOST_TEST_MESSAGE("Testing regular single period coupon after fixing...");

    Date start(12, February, 2021);
    Date end(12, August, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, true);
    testSinglePeriodCouponReplication(start, end, spread, false);
}

void SubPeriodsCouponTest::testIrregularSinglePeriodCouponAfterFixing() {
    BOOST_TEST_MESSAGE("Testing regular single period coupon after fixing...");

    Date start(12, February, 2021);
    Date end(12, June, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, true);
    testSinglePeriodCouponReplication(start, end, spread, false);
}

void SubPeriodsCouponTest::testRegularCompoundedForwardStartingCouponWithMultipleSubPeriods() {
    BOOST_TEST_MESSAGE("Testing regular forward starting coupon with multiple sub-periods...");

    Date start(15, April, 2021);
    Date end(15, April, 2022);

    Spread spread = 0.001;
    testMultipleCompoundedSubPeriodsCouponReplication(start, end, spread);
}

void SubPeriodsCouponTest::testRegularAveragedForwardStartingCouponWithMultipleSubPeriods() {
    BOOST_TEST_MESSAGE("Testing regular forward starting coupon with multiple sub-periods...");

    Date start(15, April, 2021);
    Date end(15, April, 2022);

    Spread spread = 0.001;
    testMultipleAveragedSubPeriodsCouponReplication(start, end, spread);
}

test_suite* SubPeriodsCouponTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Sub-period coupons tests");

    suite->add(
        QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testRegularSinglePeriodForwardStartingCoupon));
    suite->add(QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testRegularSinglePeriodCouponAfterFixing));
    suite->add(
        QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testIrregularSinglePeriodCouponAfterFixing));
    suite->add(QUANTLIB_TEST_CASE(
        &SubPeriodsCouponTest::testRegularCompoundedForwardStartingCouponWithMultipleSubPeriods));
    suite->add(QUANTLIB_TEST_CASE(
        &SubPeriodsCouponTest::testRegularAveragedForwardStartingCouponWithMultipleSubPeriods));

    return suite;
}