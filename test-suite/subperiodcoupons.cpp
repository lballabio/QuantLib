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
#include <ql/cashflows/subperiodcoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/cashflows/cashflows.hpp>
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
                               .backwards();
            return IborLeg(sch, euribor)
                .withNotionals(1.0)
                .withSpreads(spread)
                .withExCouponPeriod(2 * Days, calendar, businessConvention)
                .withPaymentLag(1)
                .withFixingDays(settlementDays);
        }

        ext::shared_ptr<CashFlow> createSubPeriodsCoupon(const Date& start,
                                                         const Date& end,
                                                         Spread rateSpread = 0.0,
                                                         Spread couponSpread = 0.0,
                                                         RateAveraging::Type averaging = RateAveraging::Compound) {
            Calendar paymentCalendar = euribor->fixingCalendar();
            BusinessDayConvention paymentBdc = euribor->businessDayConvention();
            Date paymentDate = paymentCalendar.advance(end, 1 * Days, paymentBdc);
            Date exCouponDate = paymentCalendar.advance(paymentDate, -2 * Days, paymentBdc);
            ext::shared_ptr<FloatingRateCoupon> cpn(new SubPeriodsCoupon(
                paymentDate, 1.0, start, end, settlementDays, euribor, 1.0, couponSpread,
                rateSpread, Date(), Date(), DayCounter(), exCouponDate));
            bool useCompoundedRate = (averaging == RateAveraging::Compound);
            if (useCompoundedRate)
                cpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new CompoundingRatePricer()));
            else
                cpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new AveragingRatePricer()));
            return cpn;
        }

        SubPeriodsLeg createSubPeriodsLeg(const Date& start,
                                          const Date& end,
                                          const Period& cpnFrequency,
                                          Spread rateSpread = 0.0,
                                          Spread couponSpread = 0.0,
                                          RateAveraging::Type averaging = RateAveraging::Compound) {
            Schedule sch = MakeSchedule()
                               .from(start)
                               .to(end)
                               .withTenor(cpnFrequency)
                               .withCalendar(euribor->fixingCalendar())
                               .withConvention(euribor->businessDayConvention())
                               .backwards();
            return SubPeriodsLeg(sch, euribor)
                .withNotionals(1.0)
                .withExCouponPeriod(2 * Days, calendar, businessConvention)
                .withPaymentLag(1)
                .withFixingDays(settlementDays)
                .withRateSpreads(rateSpread)
                .withCouponSpreads(couponSpread)
                .withAveragingMethod(averaging);
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
        Real acc = 0.0;
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
                                       RateAveraging::Type averaging) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Leg iborLeg = vars.createIborLeg(start, end, rateSpread);
    Spread couponSpread = 0.0;
    ext::shared_ptr<CashFlow> subPeriodCpn =
        vars.createSubPeriodsCoupon(start, end, rateSpread, couponSpread, averaging);

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

    Spread couponSpread = 0.0;
    ext::shared_ptr<CashFlow> subPeriodCpn = vars.createSubPeriodsCoupon(
        start, end, rateSpread, couponSpread, RateAveraging::Compound);

    const Real tolerance = 1.0e-14;

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
                                                     Spread rateSpread) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Leg iborLeg = vars.createIborLeg(start, end, rateSpread);
    
    Spread couponSpread = 0.0;
    ext::shared_ptr<CashFlow> subPeriodCpn = vars.createSubPeriodsCoupon(
        start, end, rateSpread, couponSpread, RateAveraging::Simple);

    const Real tolerance = 1.0e-14;

    Real actualPayment = subPeriodCpn->amount();
    Real expectedPayment = averagedIborLegPayment(iborLeg);

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate averaged multiple sub-period coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void testSubPeriodsLegReplication(RateAveraging::Type averaging) {
    using namespace subperiodcoupons_test;
    CommonVars vars;

    Date start(18, March, 2021);
    Date end(18, March, 2022);

    Spread rateSpread = 0.001;
    Spread couponSpread = 0.002;

    ext::shared_ptr<CashFlow> subPeriodCpn =
        vars.createSubPeriodsCoupon(start, end, rateSpread, couponSpread, averaging);

    Leg subPeriodLeg =
        vars.createSubPeriodsLeg(start, end, 1 * Years, rateSpread, couponSpread, averaging);

    const Real tolerance = 1.0e-14;

    Real actualPayment = 0.0;
    // Only one cash flow is expected with this parametrization
    std::for_each(
        subPeriodLeg.begin(), subPeriodLeg.end(),
        [&actualPayment](const ext::shared_ptr<CashFlow>& cf) { actualPayment += cf->amount(); });
    Real expectedPayment = subPeriodCpn->amount();

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate sub-period leg payments\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    averaging:    " << averaging << "\n");
}

void SubPeriodsCouponTest::testRegularSinglePeriodForwardStartingCoupon() {
    BOOST_TEST_MESSAGE("Testing regular single period forward starting coupon...");

    Date start(15, April, 2021);
    Date end(15, October, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Compound);
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Simple);
}

void SubPeriodsCouponTest::testRegularSinglePeriodCouponAfterFixing() {
    BOOST_TEST_MESSAGE("Testing regular single period coupon after fixing...");

    Date start(12, February, 2021);
    Date end(12, August, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Compound);
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Simple);
}

void SubPeriodsCouponTest::testIrregularSinglePeriodCouponAfterFixing() {
    BOOST_TEST_MESSAGE("Testing irregular single period coupon after fixing...");

    Date start(12, February, 2021);
    Date end(12, June, 2021);

    Spread spread = 0.001;
    // For a single sub-period averaging method should not matter.
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Compound);
    testSinglePeriodCouponReplication(start, end, spread, RateAveraging::Simple);
}

void SubPeriodsCouponTest::testRegularCompoundedForwardStartingCouponWithMultipleSubPeriods() {
    BOOST_TEST_MESSAGE("Testing regular forward starting coupon with multiple compounded sub-periods...");

    Date start(15, April, 2021);
    Date end(15, April, 2022);

    Spread spread = 0.001;
    testMultipleCompoundedSubPeriodsCouponReplication(start, end, spread);
}

void SubPeriodsCouponTest::testRegularAveragedForwardStartingCouponWithMultipleSubPeriods() {
    BOOST_TEST_MESSAGE("Testing regular forward starting coupon with multiple averaged sub-periods...");

    Date start(15, April, 2021);
    Date end(15, April, 2022);

    Spread spread = 0.001;
    testMultipleAveragedSubPeriodsCouponReplication(start, end, spread);
}

void SubPeriodsCouponTest::testExCouponCashFlow() {
    BOOST_TEST_MESSAGE("Testing ex-coupon cash flow...");

    using namespace subperiodcoupons_test;
    CommonVars vars;

    Date start(12, February, 2021);
    Date end(17, March, 2021);

    std::vector<ext::shared_ptr<CashFlow> > cfs{vars.createSubPeriodsCoupon(start, end)};

    Real npv = CashFlows::npv(cfs, **vars.euriborHandle, false, vars.settlement, vars.settlement);

    const Real tolerance = 1.0e-14;

    if (std::fabs(npv) > tolerance)
        BOOST_ERROR("cash flow was expected to go ex-coupon\n"
                    << std::setprecision(5) << "    calculated:    " << npv << "\n"
                    << "    expected:    " << 0.0 << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");
}

void SubPeriodsCouponTest::testSubPeriodsLegCashFlows() {
    BOOST_TEST_MESSAGE(
        "Testing sub-periods leg replication...");

    testSubPeriodsLegReplication(RateAveraging::Compound);
    testSubPeriodsLegReplication(RateAveraging::Simple);
}

void SubPeriodsCouponTest::testSubPeriodsLegConsistencyChecks() {
    BOOST_TEST_MESSAGE("Testing sub-periods leg consistency checks...");

    using namespace subperiodcoupons_test;
    CommonVars vars;

    Date start(18, March, 2021);
    Date end(18, March, 2031);

    SubPeriodsLeg subPeriodLeg =
        vars.createSubPeriodsLeg(start, end, 1 * Years);

    BOOST_CHECK_THROW(
        Leg l0(vars.createSubPeriodsLeg(start, end, 1 * Years).withNotionals(std::vector<Real>())),
        Error);
    BOOST_CHECK_THROW(Leg l1(vars.createSubPeriodsLeg(start, end, 1 * Years)
                                 .withNotionals(std::vector<Real>(11, 1.0))),
                      Error);
    BOOST_CHECK_THROW(Leg l2(vars.createSubPeriodsLeg(start, end, 1 * Years)
                                 .withFixingDays(std::vector<Natural>(11, 2))),
                      Error);
    BOOST_CHECK_THROW(Leg l3(vars.createSubPeriodsLeg(start, end, 1 * Years).withGearings(0.0)),
                      Error);
    BOOST_CHECK_THROW(Leg l4(vars.createSubPeriodsLeg(start, end, 1 * Years)
                                 .withGearings(std::vector<Real>(11, 1.0))),
                      Error);
    BOOST_CHECK_THROW(Leg l5(vars.createSubPeriodsLeg(start, end, 1 * Years)
                                 .withCouponSpreads(std::vector<Spread>(11, 0.0))),
                      Error);
    BOOST_CHECK_THROW(Leg l6(vars.createSubPeriodsLeg(start, end, 1 * Years)
                                 .withRateSpreads(std::vector<Spread>(11, 0.0))),
                      Error);
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
    suite->add(QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testExCouponCashFlow));
    suite->add(QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testSubPeriodsLegCashFlows));
    suite->add(QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::testSubPeriodsLegConsistencyChecks));

    return suite;
}
