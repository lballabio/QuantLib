/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl
 Copyright (C) 2007 Piter Dias

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

#include <ql/cashflows/fixedratecoupon.hpp>

namespace QuantLib {


    FixedRateCoupon::FixedRateCoupon(Real nominal,
                                     const Date& paymentDate,
                                     Rate rate,
                                     const DayCounter& dayCounter,
                                     const Date& accrualStartDate,
                                     const Date& accrualEndDate,
                                     const Date& refPeriodStart,
                                     const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate, accrualStartDate, accrualEndDate,
             refPeriodStart, refPeriodEnd),
      rate_(InterestRate(rate, dayCounter, Simple)),
      dayCounter_(dayCounter) {}

    FixedRateCoupon::FixedRateCoupon(Real nominal,
                                     const Date& paymentDate,
                                     const InterestRate& interestRate,
                                     const DayCounter& dayCounter,
                                     const Date& accrualStartDate,
                                     const Date& accrualEndDate,
                                     const Date& refPeriodStart,
                                     const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate, accrualStartDate, accrualEndDate,
             refPeriodStart, refPeriodEnd),
      rate_(interestRate), dayCounter_(dayCounter) {}

    Real FixedRateCoupon::amount() const {
        return nominal()*(rate_.compoundFactor(accrualStartDate_,
                                               accrualEndDate_,
                                               refPeriodStart_,
                                               refPeriodEnd_) - 1.0);
    }

    Real FixedRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal()*(rate_.compoundFactor(accrualStartDate_,
                                                   std::min(d,accrualEndDate_),
                                                   refPeriodStart_,
                                                   refPeriodEnd_) - 1.0);
        }
    }


    FixedRateLeg::FixedRateLeg(const Schedule& schedule,
                               const DayCounter& paymentDayCounter)
    : schedule_(schedule), paymentDayCounter_(paymentDayCounter),
      paymentAdjustment_(Following) {}

    FixedRateLeg& FixedRateLeg::withNotionals(Real notional) {
        notionals_ = std::vector<Real>(1,notional);
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withNotionals(
                                         const std::vector<Real>& notionals) {
        notionals_ = notionals;
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withCouponRates(Rate couponRate) {
        couponRates_.resize(1);
        couponRates_[0] = InterestRate(couponRate,
                                       paymentDayCounter_,
                                       Simple);
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withCouponRates(
                                            const InterestRate& couponRate) {
        couponRates_ = std::vector<InterestRate>(1,couponRate);
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withCouponRates(
                                       const std::vector<Rate>& couponRates) {
        couponRates_.resize(couponRates.size());
        for (Size i=0; i<couponRates.size(); ++i)
            couponRates_[i] = InterestRate(couponRates[i],
                                           paymentDayCounter_,
                                           Simple);
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withCouponRates(
                               const std::vector<InterestRate>& couponRates) {
        couponRates_ = couponRates;
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withPaymentAdjustment(
                                           BusinessDayConvention convention) {
        paymentAdjustment_ = convention;
        return *this;
    }

    FixedRateLeg& FixedRateLeg::withFirstPeriodDayCounter(
                                               const DayCounter& dayCounter) {
        firstPeriodDayCounter_ = dayCounter;
        return *this;
    }

    FixedRateLeg::operator Leg() const {

        QL_REQUIRE(!couponRates_.empty(), "no coupon rates given");
        QL_REQUIRE(!notionals_.empty(), "no notional given");

        Leg leg;

        // the following is not always correct
        Calendar calendar = schedule_.calendar();

        // first period might be short or long
        Date start = schedule_.date(0), end = schedule_.date(1);
        Date paymentDate = calendar.adjust(end, paymentAdjustment_);
        InterestRate rate = couponRates_[0];
        Real nominal = notionals_[0];
        if (schedule_.isRegular(1)) {
            QL_REQUIRE(firstPeriodDayCounter_.empty() ||
                       firstPeriodDayCounter_ == paymentDayCounter_,
                       "regular first coupon "
                       "does not allow a first-period day count");
            leg.push_back(boost::shared_ptr<CashFlow>(
                                new FixedRateCoupon(nominal, paymentDate,
                                                    rate, paymentDayCounter_,
                                                    start, end, start, end)));
        } else {
            Date ref = end - schedule_.tenor();
            ref = calendar.adjust(ref, schedule_.businessDayConvention());
            DayCounter dc = firstPeriodDayCounter_.empty() ?
                            paymentDayCounter_ :
                            firstPeriodDayCounter_;
            leg.push_back(boost::shared_ptr<CashFlow>(
                               new FixedRateCoupon(nominal, paymentDate, rate,
                                                   dc, start, end, ref, end)));
        }
        // regular periods
        for (Size i=2; i<schedule_.size()-1; ++i) {
            start = end; end = schedule_.date(i);
            paymentDate = calendar.adjust(end, paymentAdjustment_);
            if ((i-1) < couponRates_.size())
                rate = couponRates_[i-1];
            else
                rate = couponRates_.back();
            if ((i-1) < notionals_.size())
                nominal = notionals_[i-1];
            else
                nominal = notionals_.back();
            leg.push_back(boost::shared_ptr<CashFlow>(
                                new FixedRateCoupon(nominal, paymentDate,
                                                    rate, paymentDayCounter_,
                                                    start, end, start, end)));
        }
        if (schedule_.size() > 2) {
            // last period might be short or long
            Size N = schedule_.size();
            start = end; end = schedule_.date(N-1);
            paymentDate = calendar.adjust(end, paymentAdjustment_);
            if ((N-2) < couponRates_.size())
                rate = couponRates_[N-2];
            else
                rate = couponRates_.back();
            if ((N-2) < notionals_.size())
                nominal = notionals_[N-2];
            else
                nominal = notionals_.back();
            if (schedule_.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                                new FixedRateCoupon(nominal, paymentDate,
                                                    rate, paymentDayCounter_,
                                                    start, end, start, end)));
            } else {
                Date ref = start + schedule_.tenor();
                ref = calendar.adjust(ref, schedule_.businessDayConvention());
                leg.push_back(boost::shared_ptr<CashFlow>(
                                new FixedRateCoupon(nominal, paymentDate,
                                                    rate, paymentDayCounter_,
                                                    start, end, start, ref)));
            }
        }
        return leg;
    }

}

