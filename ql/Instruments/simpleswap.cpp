
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

#include <ql/Instruments/simpleswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>

namespace QuantLib {

    SimpleSwap::SimpleSwap(
                         bool payFixedRate,
                         const Date& startDate, int n, TimeUnit units,
                         const Calendar& calendar,
                         RollingConvention rollingConvention,
                         double nominal,
                         Frequency fixedFrequency,
                         Rate fixedRate,
                         bool fixedIsAdjusted,
                         const DayCounter& fixedDayCount,
                         Frequency floatingFrequency,
                         const boost::shared_ptr<Xibor>& index,
                         int indexFixingDays,
                         Spread spread,
                         const RelinkableHandle<TermStructure>& termStructure)
    : Swap(std::vector<boost::shared_ptr<CashFlow> >(),
           std::vector<boost::shared_ptr<CashFlow> >(),
           termStructure),
      payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
      nominal_(nominal) {

        Date maturity = calendar.roll(startDate.plus(n,units),
                                      rollingConvention);

        Schedule fixedSchedule = 
            MakeSchedule(calendar,startDate,maturity,
                         fixedFrequency,rollingConvention,
                         fixedIsAdjusted);
        Schedule floatSchedule =
            MakeSchedule(calendar,startDate,maturity,
                         floatingFrequency,rollingConvention,
                         true);

        std::vector<boost::shared_ptr<CashFlow> > fixedLeg =
            FixedRateCouponVector(fixedSchedule, 
                                  std::vector<double>(1,nominal), 
                                  std::vector<Rate>(1,fixedRate), 
                                  fixedDayCount);
        std::vector<boost::shared_ptr<CashFlow> > floatingLeg =
            FloatingRateCouponVector(floatSchedule,
                                     std::vector<double>(1,nominal),
                                     index, indexFixingDays, 
                                     std::vector<Spread>(1,spread));
        std::vector<boost::shared_ptr<CashFlow> >::const_iterator i;
        for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
            registerWith(*i);

        if (payFixedRate_) {
            firstLeg_ = fixedLeg;
            secondLeg_ = floatingLeg;
        } else {
            firstLeg_ = floatingLeg;
            secondLeg_ = fixedLeg;
        }
    }

    SimpleSwap::SimpleSwap(
                         bool payFixedRate,
                         double nominal,
                         const Schedule& fixedSchedule,
                         Rate fixedRate,
                         const DayCounter& fixedDayCount,
                         const Schedule& floatSchedule,
                         const boost::shared_ptr<Xibor>& index,
                         int indexFixingDays,
                         Spread spread,
                         const RelinkableHandle<TermStructure>& termStructure)
    : Swap(std::vector<boost::shared_ptr<CashFlow> >(),
           std::vector<boost::shared_ptr<CashFlow> >(),
           termStructure),
      payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
      nominal_(nominal) {

        std::vector<boost::shared_ptr<CashFlow> > fixedLeg =
            FixedRateCouponVector(fixedSchedule,
                                  std::vector<double>(1,nominal), 
                                  std::vector<Rate>(1,fixedRate), 
                                  fixedDayCount);
        std::vector<boost::shared_ptr<CashFlow> > floatingLeg =
            FloatingRateCouponVector(floatSchedule,
                                     std::vector<double>(1,nominal),
                                     index, indexFixingDays, 
                                     std::vector<Spread>(1,spread));
        std::vector<boost::shared_ptr<CashFlow> >::const_iterator i;
        for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
            registerWith(*i);

        if (payFixedRate_) {
            firstLeg_ = fixedLeg;
            secondLeg_ = floatingLeg;
        } else {
            firstLeg_ = floatingLeg;
            secondLeg_ = fixedLeg;
        }
    }


    void SimpleSwap::setupArguments(Arguments* args) const {
        SimpleSwap::arguments* arguments =
            dynamic_cast<SimpleSwap::arguments*>(args);

        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->payFixed = payFixedRate_;
        arguments->nominal = nominal_;

        Date settlement = termStructure_->referenceDate();
        DayCounter counter = termStructure_->dayCounter();
        Size i;

        const std::vector<boost::shared_ptr<CashFlow> >& fixedCoupons = 
            fixedLeg();

        arguments->fixedResetTimes = arguments->fixedPayTimes =
            std::vector<Time>(fixedCoupons.size());
        arguments->fixedCoupons = std::vector<double>(fixedCoupons.size());

        for (i=0; i<fixedCoupons.size(); i++) {
            boost::shared_ptr<FixedRateCoupon> coupon = 
                boost::dynamic_pointer_cast<FixedRateCoupon>(fixedCoupons[i]);

            Time time = counter.yearFraction(settlement, coupon->date());
            arguments->fixedPayTimes[i] = time;
            time = counter.yearFraction(settlement, 
                                        coupon->accrualStartDate());
            arguments->fixedResetTimes[i] = time;
            arguments->fixedCoupons[i] = coupon->amount();
        }

        const std::vector<boost::shared_ptr<CashFlow> >& floatingCoupons = 
            floatingLeg();

        arguments->floatingResetTimes = arguments->floatingPayTimes =
            arguments->floatingAccrualTimes = 
            std::vector<Time>(floatingCoupons.size());
        arguments->floatingSpreads =
            std::vector<Spread>(floatingCoupons.size());

        for (i=0; i<floatingCoupons.size(); i++) {
            boost::shared_ptr<FloatingRateCoupon> coupon = 
                boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                          floatingCoupons[i]);

            Date resetDate = coupon->accrualStartDate(); // already rolled
            Time time = counter.yearFraction(settlement, resetDate);
            arguments->floatingResetTimes[i] = time;
            time = counter.yearFraction(settlement, coupon->date());
            arguments->floatingPayTimes[i] = time;
            arguments->floatingAccrualTimes[i] = coupon->accrualPeriod();
            arguments->floatingSpreads[i] = coupon->spread();
        }
    }


    void SimpleSwap::arguments::validate() const {
        QL_REQUIRE(nominal != Null<Real>(),
                   "nominal null or not set");
        QL_REQUIRE(fixedResetTimes.size() == fixedPayTimes.size(), 
                   "number of fixed start times different from "
                   "number of fixed payment times");
        QL_REQUIRE(fixedPayTimes.size() == fixedCoupons.size(), 
                   "number of fixed payment times different from "
                   "number of fixed coupon amounts");
        QL_REQUIRE(floatingResetTimes.size() == floatingPayTimes.size(), 
                   "number of floating start times different from "
                   "number of floating payment times");
        QL_REQUIRE(floatingAccrualTimes.size() == floatingPayTimes.size(), 
                   "number of floating accrual times different from "
                   "number of floating payment times");
        QL_REQUIRE(floatingSpreads.size() == floatingPayTimes.size(), 
                   "number of floating spreads different from "
                   "number of floating payment times");
    }

}

