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

            today = calendar.adjust(Date(15, March, 2021));
            Settings::instance().evaluationDate() = today;
            settlement = calendar.advance(today, settlementDays, Days);

            euriborHandle.linkTo(flatRate(settlement, 0.007, dayCount));
        }
    };

    Leg createIborLeg(const Date& start, 
        const Date& end, 
        bool isInArrears = false,
        Spread spread = 0.0)
    {
        CommonVars vars;
        Schedule sch = MakeSchedule()
                           .from(start)
                           .to(end)
                           .withTenor(vars.euribor->tenor())
                           .withCalendar(vars.euribor->fixingCalendar())
                           .withConvention(vars.euribor->businessDayConvention())
                           .backwards()
                           .endOfMonth(vars.euribor->endOfMonth());
        return IborLeg(sch, vars.euribor)
            .withNotionals(1.0)
            .withSpreads(spread)
            .withExCouponPeriod(2 * Days, vars.calendar, vars.businessConvention)
            .withPaymentLag(1)
            .inArrears(isInArrears)
            .withFixingDays(vars.settlementDays);
    }

    Real sumLegPayments(const Leg& leg)
    {
        Real payments = 0.0;
        std::for_each(leg.begin(), leg.end(),
                      [&](const ext::shared_ptr<CashFlow>& cf) { payments += cf->amount(); });
        return payments;
    }

    ext::shared_ptr<CashFlow> createSubPeriodsCoupon(const Date& start,
                                                    const Date& end,
                                                    bool isInArrears = false,
                                                    Spread spread = 0.0) {
        CommonVars vars;
        Date paymentDate = vars.calendar.advance(end, 1 * Days, vars.businessConvention);
        Date exCouponDate = vars.calendar.advance(paymentDate, -2 * Days, vars.businessConvention);
        ext::shared_ptr<FloatingRateCoupon> cpn(new SubPeriodsCoupon(
            paymentDate, 1.0, start, end, vars.settlementDays, vars.euribor, 1.0, spread, 0.0,
            Date(), Date(), vars.dayCount, isInArrears, exCouponDate));
        cpn->setPricer(ext::shared_ptr<FloatingRateCouponPricer>(new CompoundingRatePricer()));
        return cpn;
    }
}

void SubPeriodsCouponTest::test() {
    BOOST_TEST_MESSAGE("...");

    using namespace subperiodcoupons_test;

    Date start(15, April, 2021);
    Date end(15, September, 2021);

    Leg iborLeg = createIborLeg(start, end);
    ext::shared_ptr<CashFlow> subPeriodCpn = createSubPeriodsCoupon(start, end);

    Real tolerance = 1.0e-10;

    Real actualPayment = subPeriodCpn->amount();
    Real expectedPayment = sumLegPayments(iborLeg);

    if (std::fabs(actualPayment - expectedPayment) > tolerance)
        BOOST_ERROR("unable to replicate sub-periods coupon payment\n"
                    << std::setprecision(5) << "    calculated:    " << actualPayment << "\n"
                    << "    expected:    " << expectedPayment << "\n"
                    << "    start:    " << start << "\n"
                    << "    end:    " << end << "\n");


}

test_suite* SubPeriodsCouponTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Sub-period coupons tests");

    suite->add(QUANTLIB_TEST_CASE(&SubPeriodsCouponTest::test));

    return suite;
}