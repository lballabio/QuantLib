
/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/shortfloatingcoupon.hpp>

namespace QuantLib {

    std::vector<boost::shared_ptr<CashFlow> > 
    FixedRateCouponVector(const Schedule& schedule,
                          BusinessDayConvention paymentAdjustment,
                          const std::vector<Real>& nominals,
                          const std::vector<Rate>& couponRates,
                          const DayCounter& dayCount, 
                          const DayCounter& firstPeriodDayCount) {

        QL_REQUIRE(couponRates.size() != 0, 
                   "unspecified coupon rates (size=0)");
        QL_REQUIRE(nominals.size() != 0, 
                   "unspecified nominals (size=0)");
        QL_REQUIRE(paymentAdjustment != Unadjusted,
                   "invalid business-day convention "
                   "for payment-date adjustment");

        std::vector<boost::shared_ptr<CashFlow> > leg;
        Calendar calendar = schedule.calendar();

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        Rate rate = couponRates[0];
        Real nominal = nominals[0];
        if (schedule.isRegular(1)) {
            QL_REQUIRE(firstPeriodDayCount.isNull() ||
                       firstPeriodDayCount == dayCount,
                       "regular first coupon "
                       "does not allow a first-period day count");
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate, dayCount,
                                    start, end, start, end)));
        } else {
            Date reference = end.plusMonths(-12/schedule.frequency());
            reference = calendar.adjust(reference,
                                        schedule.businessDayConvention());
            DayCounter dc = firstPeriodDayCount.isNull() ?
                            dayCount :
                            firstPeriodDayCount;
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate, 
                                    dc, start, end, reference, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; i++) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((i-1) < couponRates.size())
                rate = couponRates[i-1];
            else
                rate = couponRates.back();
            if ((i-1) < nominals.size())
                nominal = nominals[i-1];
            else
                nominal = nominals.back();
            leg.push_back(boost::shared_ptr<CashFlow>(
                new FixedRateCoupon(nominal, paymentDate, rate, dayCount, 
                                    start, end, start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            Size N = schedule.size();
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((N-2) < couponRates.size())
                rate = couponRates[N-2];
            else
                rate = couponRates.back();
            if ((N-2) < nominals.size())
                nominal = nominals[N-2];
            else
                nominal = nominals.back();
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate, 
                                        rate, dayCount, 
                                        start, end, start, end)));
            } else {
                Date reference = 
                    start.plusMonths(12/schedule.frequency());
                reference = calendar.adjust(reference,
                                            schedule.businessDayConvention());
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new FixedRateCoupon(nominal, paymentDate, 
                                        rate, dayCount, 
                                        start, end, start, reference)));
            }
        }
        return leg;
    }

    std::vector<boost::shared_ptr<CashFlow> > 
    FloatingRateCouponVector(const Schedule& schedule,
                             BusinessDayConvention paymentAdjustment,
                             const std::vector<Real>& nominals,
                             const boost::shared_ptr<Xibor>& index, 
                             Integer fixingDays,
                             const std::vector<Spread>& spreads) {

        QL_REQUIRE(nominals.size() != 0, "unspecified nominals");
        QL_REQUIRE(paymentAdjustment != Unadjusted,
                   "invalid business-day convention "
                   "for payment-date adjustment");

        std::vector<boost::shared_ptr<CashFlow> > leg;
        Calendar calendar = schedule.calendar();

        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        Spread spread;
        if (spreads.size() > 0)
            spread = spreads[0];
        else
            spread = 0.0;
        Real nominal = nominals[0];
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new ParCoupon(nominal, paymentDate, index, start, end, 
                              fixingDays, spread, start, end)));
        } else {
            Date reference = end.plusMonths(-12/schedule.frequency());
            reference =
                calendar.adjust(reference,schedule.businessDayConvention());
            leg.push_back(boost::shared_ptr<CashFlow>(
                new ShortFloatingRateCoupon(nominal, paymentDate, 
                                            index, start, end, 
                                            fixingDays, spread, 
                                            reference, end)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; i++) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
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
            leg.push_back(boost::shared_ptr<CashFlow>(
                new ParCoupon(nominal, paymentDate, index, start, end, 
                              fixingDays, spread, start, end)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            Size N = schedule.size();
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
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
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new ParCoupon(nominal, paymentDate, index, start, end, 
                                  fixingDays, spread, start, end)));
            } else {
                Date reference = 
                    start.plusMonths(12/schedule.frequency());
                reference =
                    calendar.adjust(reference,
                                    schedule.businessDayConvention());
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new ShortFloatingRateCoupon(nominal, paymentDate, 
                                                index, start, end, 
                                                fixingDays, spread, 
                                                start, reference)));
            }
        }
        return leg;
    }

}

