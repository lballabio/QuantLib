/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2007 StatPro Italia srl
 Copyright (C) 2007 Piter Dias
 Copyright (C) 2010 Ferdinando Ametrano

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

/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate
*/

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include <ql/cashflows/coupon.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/interestrate.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! %Coupon paying a fixed interest rate
    class FixedRateCoupon : public Coupon {
      public:
        //! \name constructors
        //@{
        FixedRateCoupon(const Date& paymentDate,
                        Real nominal,
                        Rate rate,
                        const DayCounter& dayCounter,
                        const Date& accrualStartDate,
                        const Date& accrualEndDate,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date(),
                        const Date& exCouponDate = Date());
        FixedRateCoupon(const Date& paymentDate,
                        Real nominal,
                        const InterestRate& interestRate,
                        const Date& accrualStartDate,
                        const Date& accrualEndDate,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date(),
                        const Date& exCouponDate = Date());
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const;
        //@}
        //! \name Coupon interface
        //@{
        Rate rate() const { return rate_; }
        InterestRate interestRate() const { return rate_; }
        DayCounter dayCounter() const { return rate_.dayCounter(); }
        Real accruedAmount(const Date&) const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        InterestRate rate_;
    };



    //! helper class building a sequence of fixed rate coupons
    class FixedRateLeg {
      public:
        FixedRateLeg(const Schedule& schedule);
        FixedRateLeg& withNotionals(Real);
        FixedRateLeg& withNotionals(const std::vector<Real>&);
        FixedRateLeg& withCouponRates(Rate,
                                      const DayCounter& paymentDayCounter,
                                      Compounding comp = Simple,
                                      Frequency freq = Annual);
        FixedRateLeg& withCouponRates(const std::vector<Rate>&,
                                      const DayCounter& paymentDayCounter,
                                      Compounding comp = Simple,
                                      Frequency freq = Annual);
        FixedRateLeg& withCouponRates(const InterestRate&);
        FixedRateLeg& withCouponRates(const std::vector<InterestRate>&);
        FixedRateLeg& withPaymentAdjustment(BusinessDayConvention);
        FixedRateLeg& withFirstPeriodDayCounter(const DayCounter&);
        FixedRateLeg& withPaymentCalendar(const Calendar&);
        FixedRateLeg& withExCouponPeriod(const Period&,
                                         const Calendar&,
                                         BusinessDayConvention,
                                         bool endOfMonth);
        operator Leg() const;
      private:
        Schedule schedule_;
        Calendar calendar_;
        std::vector<Real> notionals_;
        std::vector<InterestRate> couponRates_;
        DayCounter firstPeriodDC_;
        BusinessDayConvention paymentAdjustment_;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_;
        bool exCouponEndOfMonth_;
    };

    inline void FixedRateCoupon::accept(AcyclicVisitor& v) {
        Visitor<FixedRateCoupon>* v1 =
            dynamic_cast<Visitor<FixedRateCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

}


#endif
