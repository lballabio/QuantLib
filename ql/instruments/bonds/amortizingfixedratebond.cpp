/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Simon Ibbotson

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

#include <ql/instruments/bonds/amortizingfixedratebond.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/cashflows/simplecashflow.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    AmortizingFixedRateBond::AmortizingFixedRateBond(
                                      Natural settlementDays,
                                      const std::vector<Real>& notionals,
                                      const Schedule& schedule,
                                      const std::vector<Rate>& coupons,
                                      const DayCounter& accrualDayCounter,
                                      BusinessDayConvention paymentConvention,
                                      const Date& issueDate,
                                      const Period& exCouponPeriod,
                                      const Calendar& exCouponCalendar,
                                      const BusinessDayConvention exCouponConvention,
                                      bool exCouponEndOfMonth,
                                      const std::vector<Real>& redemptions,
                                      Natural paymentLag)
    : Bond(settlementDays, schedule.calendar(), issueDate),
      frequency_(schedule.tenor().frequency()),
      dayCounter_(accrualDayCounter) {

        maturityDate_ = schedule.endDate();

        cashflows_ = FixedRateLeg(schedule)
            .withNotionals(notionals)
            .withCouponRates(coupons, accrualDayCounter)
            .withPaymentAdjustment(paymentConvention)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth)
            .withPaymentLag(paymentLag);

        addRedemptionsToCashflows(redemptions);

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
    }


    AmortizingFixedRateBond::AmortizingFixedRateBond(
                                      Natural settlementDays,
                                      const Calendar& calendar,
                                      Real initialFaceAmount,
                                      const Date& startDate,
                                      const Period& bondTenor,
                                      const Frequency& sinkingFrequency,
                                      const Rate coupon,
                                      const DayCounter& accrualDayCounter,
                                      BusinessDayConvention paymentConvention,
                                      const Date& issueDate)
    : Bond(settlementDays, calendar, issueDate),
      frequency_(sinkingFrequency),
      dayCounter_(accrualDayCounter) {

        QL_REQUIRE(bondTenor.length() > 0,
                   "bond tenor must be positive. "
                   << bondTenor << " is not allowed.");
        maturityDate_ = startDate + bondTenor;

        cashflows_ =
            FixedRateLeg(sinkingSchedule(startDate, bondTenor,
                                         sinkingFrequency, calendar))
            .withNotionals(sinkingNotionals(bondTenor,
                                            sinkingFrequency, coupon,
                                            initialFaceAmount))
            .withCouponRates(coupon, accrualDayCounter)
            .withPaymentAdjustment(paymentConvention);

        addRedemptionsToCashflows();
    }

  
    AmortizingFixedRateBond::AmortizingFixedRateBond(
                                      Natural settlementDays,
                                      const std::vector<Real>& notionals,
                                      const Schedule& schedule,
                                      const std::vector<InterestRate>& coupons,
                                      BusinessDayConvention paymentConvention,
                                      const Date& issueDate,
                                      const Calendar& paymentCalendar,
                                      const Period& exCouponPeriod,
                                      const Calendar& exCouponCalendar,
                                      const BusinessDayConvention exCouponConvention,
                                      bool exCouponEndOfMonth)
    : Bond(settlementDays,
        paymentCalendar==Calendar() ? schedule.calendar() : paymentCalendar,
        issueDate),
      frequency_(schedule.tenor().frequency()),
      dayCounter_(coupons[0].dayCounter()) {

        maturityDate_ = schedule.endDate();

        cashflows_ = FixedRateLeg(schedule)
            .withNotionals(notionals)
            .withCouponRates(coupons)
            .withPaymentAdjustment(paymentConvention)
            .withExCouponPeriod(exCouponPeriod,
                                exCouponCalendar,
                                exCouponConvention,
                                exCouponEndOfMonth);

        addRedemptionsToCashflows();

        QL_ENSURE(!cashflows().empty(), "bond with no cashflows!");
    }


    Schedule sinkingSchedule(const Date& startDate,
                             const Period& bondLength,
                             const Frequency& frequency,
                             const Calendar& paymentCalendar) {
        Date maturityDate = startDate + bondLength;
        Schedule retVal(startDate, maturityDate, Period(frequency),
                        paymentCalendar, Unadjusted, Unadjusted,
                        DateGeneration::Backward, false);
        return retVal;
    }

    namespace  {

        std::pair<Integer,Integer> daysMinMax(const Period& p) {
            switch (p.units()) {
              case Days:
                return std::make_pair(p.length(), p.length());
              case Weeks:
                return std::make_pair(7*p.length(), 7*p.length());
              case Months:
                return std::make_pair(28*p.length(), 31*p.length());
              case Years:
                return std::make_pair(365*p.length(), 366*p.length());
              default:
                QL_FAIL("unknown time unit (" << Integer(p.units()) << ")");
            }
        }

        bool isSubPeriod(const Period& subPeriod,
                         const Period& superPeriod,
                         Integer& numSubPeriods) {

            std::pair<Integer, Integer> superDays(daysMinMax(superPeriod));
            std::pair<Integer, Integer> subDays(daysMinMax(subPeriod));

            //obtain the approximate time ratio
            Real minPeriodRatio =
                ((Real)superDays.first)/((Real)subDays.second);
            Real maxPeriodRatio =
                ((Real)superDays.second)/((Real)subDays.first);
            auto lowRatio = static_cast<Integer>(std::floor(minPeriodRatio));
            auto highRatio = static_cast<Integer>(std::ceil(maxPeriodRatio));

            try {
                for(Integer i=lowRatio; i <= highRatio; ++i) {
                    Period testPeriod = subPeriod * i;
                    if(testPeriod == superPeriod) {
                        numSubPeriods = i;
                        return true;
                    }
                }
            } catch(Error&) {
                return false;
            }

            return false;
        }

    }

    std::vector<Real> sinkingNotionals(const Period& bondLength,
                                       const Frequency& sinkingFrequency,
                                       Rate couponRate,
                                       Real initialNotional) {
        Integer nPeriods;
        QL_REQUIRE(isSubPeriod(Period(sinkingFrequency), bondLength, nPeriods),
                   "Bond frequency is incompatible with the maturity tenor");

        std::vector<Real> notionals(nPeriods+1);
        notionals.front() = initialNotional;
        Real coupon = couponRate / static_cast<Real>(sinkingFrequency);
        Real compoundedInterest = 1.0;
        Real totalValue = std::pow(1.0+coupon, nPeriods);
        for (Size i = 0; i < (Size)nPeriods-1; ++i) {
            compoundedInterest *= (1.0 + coupon);
            Real currentNotional = 0.0;
            if(coupon < 1.0e-12) {
                currentNotional = initialNotional*(1.0 - (i+1.0)/nPeriods);
            } else {
                currentNotional =
                    initialNotional*(compoundedInterest - (compoundedInterest-1.0)/(1.0 - 1.0/totalValue));
            }
            notionals[i+1] = currentNotional;
        }
        notionals.back() = 0.0;

        return notionals;
    }

}
