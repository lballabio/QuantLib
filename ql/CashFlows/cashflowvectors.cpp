
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file cashflowvectors.cpp
    \brief Cash flow vector builders

    \fullpath
    ql/CashFlows/%cashflowvectors.cpp
*/

// $Id$

#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/scheduler.hpp>

namespace QuantLib {

    using Indexes::Xibor;

    namespace CashFlows {

        FixedRateCouponVector::FixedRateCouponVector(
          const std::vector<double>& nominals,
          const std::vector<Rate>& couponRates,
          const Date& startDate, const Date& endDate,
          int frequency, const Calendar& calendar,
          RollingConvention rollingConvention, bool isAdjusted,
          const DayCounter& dayCount, const DayCounter& firstPeriodDayCount,
          const Date& stubDate) {
            QL_REQUIRE(couponRates.size() != 0, "unspecified coupon rates");
            QL_REQUIRE(nominals.size() != 0, "unspecified nominals");
            Scheduler scheduler(calendar, startDate, endDate, frequency,
                rollingConvention, isAdjusted, stubDate);
            // first period might be short or long
            Date start = scheduler.date(0), end = scheduler.date(1);
            Rate rate = couponRates[0];
            double nominal = nominals[0];
            if (scheduler.isRegular(1)) {
                QL_REQUIRE(dayCount == firstPeriodDayCount,
                    "regular first bond coupon "
                    "does not allow a first period day count");
                push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, rate, calendar,
                        rollingConvention, dayCount,
                        start, end, start, end)));
            } else {
                Date reference = end.plusMonths(-12/frequency);
                if (isAdjusted)
                    reference =
                        calendar.roll(reference,rollingConvention);
                push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, rate, calendar,
                        rollingConvention, firstPeriodDayCount,
                        start, end, reference, end)));
            }
            // regular periods
            for (Size i=2; i<scheduler.size()-1; i++) {
                start = end; end = scheduler.date(i);
                if ((i-1) < couponRates.size())
                    rate = couponRates[i-1];
                else
                    rate = couponRates.back();
                if ((i-1) < nominals.size())
                    nominal = nominals[i-1];
                else
                    nominal = nominals.back();
                push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, rate, calendar,
                        rollingConvention, dayCount, start, end,
                        start, end)));
            }
            if (scheduler.size() > 2) {
                // last period might be short or long
                Size N = scheduler.size();
                start = end; end = scheduler.date(N-1);
                if ((N-2) < couponRates.size())
                    rate = couponRates[N-2];
                else
                    rate = couponRates.back();
                if ((N-2) < nominals.size())
                    nominal = nominals[N-2];
                else
                    nominal = nominals.back();
                if (scheduler.isRegular(N-1)) {
                    push_back(Handle<CashFlow>(
                        new FixedRateCoupon(nominal, rate, calendar,
                            rollingConvention, dayCount, start, end,
                            start, end)));
                } else {
                    Date reference = start.plusMonths(12/frequency);
                    if (isAdjusted)
                        reference =
                            calendar.roll(reference,rollingConvention);
                    push_back(Handle<CashFlow>(
                        new FixedRateCoupon(nominal, rate, calendar,
                            rollingConvention, dayCount, start, end,
                            start, reference)));
                }
            }
        }


        FloatingRateCouponVector::FloatingRateCouponVector(
          const std::vector<double>& nominals,
          const Date& startDate, const Date& endDate,
          int frequency, const Calendar& calendar,
          RollingConvention rollingConvention,
          const RelinkableHandle<TermStructure>& termStructure,
          const Handle<Xibor>& index, int fixingDays,
          const std::vector<Spread>& spreads,
          const Date& stubDate) {
            QL_REQUIRE(nominals.size() != 0, "unspecified nominals");

            /* the following precondition is to be removed when an
               algorithm for the fixing of the short coupon is implemented */
            QL_REQUIRE(stubDate == Date(),
                "short/long floating coupons are currently disabled");

            Scheduler scheduler(calendar, startDate, endDate, frequency,
                rollingConvention, true, stubDate);
            // first period might be short or long
            Date start = scheduler.date(0), end = scheduler.date(1);
            Spread spread;
            if (spreads.size() > 0)
                spread = spreads[0];
            else
                spread = 0.0;
            double nominal = nominals[0];
            if (scheduler.isRegular(1)) {
                push_back(Handle<CashFlow>(
                    new FloatingRateCoupon(nominal, index, termStructure,
                        start, end, fixingDays, spread, start, end)));
            } else {
                Date reference = end.plusMonths(-12/frequency);
                reference =
                    calendar.roll(reference,rollingConvention);
                push_back(Handle<CashFlow>(
                    new FloatingRateCoupon(nominal, index, termStructure,
                        start, end, fixingDays, spread, reference, end)));
            }
            // regular periods
            for (Size i=2; i<scheduler.size()-1; i++) {
                start = end; end = scheduler.date(i);
                if ((i-1) < spreads.size())
                    spread = spreads[i-1];
                else if (spreads.size() > 0)
                    spread = spreads.back();
                else
                    spread = 0.0;
                if ((i-1) < nominals.size())
                    nominal = nominals[i-1];
                else
                    nominal = nominals.back();
                push_back(Handle<CashFlow>(
                    new FloatingRateCoupon(nominal, index, termStructure,
                        start, end, fixingDays, spread, start, end)));
            }
            if (scheduler.size() > 2) {
                // last period might be short or long
                Size N = scheduler.size();
                start = end; end = scheduler.date(N-1);
                if ((N-2) < spreads.size())
                    spread = spreads[N-2];
                else if (spreads.size() > 0)
                    spread = spreads.back();
                else
                    spread = 0.0;
                if ((N-2) < nominals.size())
                    nominal = nominals[N-2];
                else
                    nominal = nominals.back();
                if (scheduler.isRegular(N-1)) {
                    push_back(Handle<CashFlow>(
                        new FloatingRateCoupon(nominal, index, termStructure,
                            start, end, fixingDays, spread, start, end)));
                } else {
                    Date reference = start.plusMonths(12/frequency);
                    reference =
                        calendar.roll(reference,rollingConvention);
                    push_back(Handle<CashFlow>(
                        new FloatingRateCoupon(nominal, index, termStructure,
                            start, end, fixingDays, spread, start, reference)));
                }
            }
        }

    }

}

