
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cashflowvectors.cpp
    \brief Cash flow vector builders

    \fullpath
    ql/CashFlows/%cashflowvectors.cpp
*/

// $Id$

#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/shortfloatingcoupon.hpp>
#include <ql/scheduler.hpp>

namespace QuantLib {

    using Indexes::Xibor;

    namespace CashFlows {

        std::vector<Handle<CashFlow> > FixedRateCouponVector(
          const std::vector<double>& nominals,
          const std::vector<Rate>& couponRates,
          const Date& startDate, const Date& endDate,
          int frequency, const Calendar& calendar,
          RollingConvention rollingConvention, bool isAdjusted,
          const DayCounter& dayCount, const DayCounter& firstPeriodDayCount,
          const Date& stubDate) {
            QL_REQUIRE(couponRates.size() != 0, 
                       "FixedRateCouponVector: unspecified coupon rates(size=0)");
            QL_REQUIRE(nominals.size() != 0, 
                       "FixedRateCouponVector: unspecified nominals()size=0");

            std::vector<Handle<CashFlow> > leg;
            Scheduler scheduler(calendar, startDate, endDate, frequency,
                rollingConvention, isAdjusted, stubDate);
            // first period might be short or long
            Date start = scheduler.date(0), end = scheduler.date(1);
            Date paymentDate = calendar.roll(end,rollingConvention);
            Rate rate = couponRates[0];
            double nominal = nominals[0];
            if (scheduler.isRegular(1)) {
                QL_REQUIRE(dayCount == firstPeriodDayCount,
                    "regular first coupon "
                    "does not allow a first period day count");
                leg.push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate, rate, dayCount,
                        start, end, start, end)));
            } else {
                Date reference = end.plusMonths(-12/frequency);
                if (isAdjusted)
                    reference =
                        calendar.roll(reference,rollingConvention);
                leg.push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate, rate, 
                        firstPeriodDayCount, start, end, reference, end)));
            }
            // regular periods
            for (Size i=2; i<scheduler.size()-1; i++) {
                start = end; end = scheduler.date(i);
                paymentDate = calendar.roll(end,rollingConvention);
                if ((i-1) < couponRates.size())
                    rate = couponRates[i-1];
                else
                    rate = couponRates.back();
                if ((i-1) < nominals.size())
                    nominal = nominals[i-1];
                else
                    nominal = nominals.back();
                leg.push_back(Handle<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate, rate, dayCount, 
                        start, end, start, end)));
            }
            if (scheduler.size() > 2) {
                // last period might be short or long
                Size N = scheduler.size();
                start = end; end = scheduler.date(N-1);
                paymentDate = calendar.roll(end,rollingConvention);
                if ((N-2) < couponRates.size())
                    rate = couponRates[N-2];
                else
                    rate = couponRates.back();
                if ((N-2) < nominals.size())
                    nominal = nominals[N-2];
                else
                    nominal = nominals.back();
                if (scheduler.isRegular(N-1)) {
                    leg.push_back(Handle<CashFlow>(
                        new FixedRateCoupon(nominal, paymentDate, rate, dayCount, 
                            start, end, start, end)));
                } else {
                    Date reference = start.plusMonths(12/frequency);
                    if (isAdjusted)
                        reference =
                            calendar.roll(reference,rollingConvention);
                    leg.push_back(Handle<CashFlow>(
                        new FixedRateCoupon(nominal, paymentDate, rate, dayCount, 
                            start, end, start, reference)));
                }
            }
            return leg;
        }


        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
          const std::vector<double>& nominals,
          const Date& startDate, const Date& endDate,
          int frequency, const Calendar& calendar,
          RollingConvention rollingConvention,
          const Handle<Xibor>& index, int fixingDays,
          const std::vector<Spread>& spreads,
          const Date& stubDate) {
            QL_REQUIRE(nominals.size() != 0, "unspecified nominals");

            std::vector<Handle<CashFlow> > leg;
            Scheduler scheduler(calendar, startDate, endDate, frequency,
                rollingConvention, true, stubDate);
            // first period might be short or long
            Date start = scheduler.date(0), end = scheduler.date(1);
            Date paymentDate = calendar.roll(end,rollingConvention);
            Spread spread;
            if (spreads.size() > 0)
                spread = spreads[0];
            else
                spread = 0.0;
            double nominal = nominals[0];
            if (scheduler.isRegular(1)) {
                leg.push_back(Handle<CashFlow>(
                    new FloatingRateCoupon(nominal, paymentDate, index, start, end, 
                                           fixingDays, spread, start, end)));
            } else {
                Date reference = end.plusMonths(-12/frequency);
                reference =
                    calendar.roll(reference,rollingConvention);
                leg.push_back(Handle<CashFlow>(
                    new ShortFloatingRateCoupon(nominal, paymentDate, index, start, end, 
                                                fixingDays, spread, 
                                                reference, end)));
            }
            // regular periods
            for (Size i=2; i<scheduler.size()-1; i++) {
                start = end; end = scheduler.date(i);
                paymentDate = calendar.roll(end,rollingConvention);
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
                leg.push_back(Handle<CashFlow>(
                    new FloatingRateCoupon(nominal, paymentDate, index, start, end, 
                                           fixingDays, spread, start, end)));
            }
            if (scheduler.size() > 2) {
                // last period might be short or long
                Size N = scheduler.size();
                start = end; end = scheduler.date(N-1);
                paymentDate = calendar.roll(end,rollingConvention);
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
                    leg.push_back(Handle<CashFlow>(
                        new FloatingRateCoupon(nominal, paymentDate, index, start, end, 
                                               fixingDays, spread, 
                                               start, end)));
                } else {
                    Date reference = start.plusMonths(12/frequency);
                    reference =
                        calendar.roll(reference,rollingConvention);
                    leg.push_back(Handle<CashFlow>(
                        new ShortFloatingRateCoupon(nominal, paymentDate, index, start, end, 
                                                    fixingDays, spread, 
                                                    start, reference)));
                }
            }
            return leg;
        }

    }

}

