/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file floatingratecoupon.hpp
    \brief Coupon paying a variable rate
*/

#ifndef quantlib_floating_rate_coupon_hpp
#define quantlib_floating_rate_coupon_hpp

#include <ql/CashFlows/coupon.hpp>
#include <ql/Utilities/null.hpp>

namespace QuantLib {

    //! %Coupon paying a variable rate
    /*! \warning This class does not perform any date adjustment,
                 i.e., the start and end date passed upon construction
                 should be already rolled to a business day.
    */
    class FloatingRateCoupon : public Coupon {
      public:
        FloatingRateCoupon(Real nominal, const Date& paymentDate,
                           const Date& startDate, const Date& endDate,
                           Integer fixingDays, Spread spread = 0.0,
                           const Date& refPeriodStart = Date(),
                           const Date& refPeriodEnd = Date());
        //! \name Coupon interface
        //@{
        Rate rate() const;
        Real accruedAmount(const Date&) const;
        //@}
        //! \name Inspectors
        //@{
        //! fixing days
        Integer fixingDays() const;
        //! spread paid over the fixing of the underlying index
        virtual Spread spread() const;
        //! fixing of the underlying index
        virtual Rate indexFixing() const = 0;
        //! fixing date
        virtual Date fixingDate() const = 0;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        //! convexity adjustment for the given index fixing
        virtual Rate convexityAdjustment(Rate fixing) const;
        Integer fixingDays_;
        Spread spread_;
    };


    // inline definitions

    inline FloatingRateCoupon::FloatingRateCoupon(
                         Real nominal, const Date& paymentDate,
                         const Date& startDate, const Date& endDate,
                         Integer fixingDays, Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      fixingDays_(fixingDays), spread_(spread) {}

    inline Rate FloatingRateCoupon::rate() const {
        Rate f = indexFixing();
        return f + convexityAdjustment(f) + spread();
    }

    inline Real FloatingRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal() * rate() *
                dayCounter().yearFraction(accrualStartDate_,
                                          std::min(d,accrualEndDate_),
                                          refPeriodStart_,
                                          refPeriodEnd_);
        }
    }

    inline Integer FloatingRateCoupon::fixingDays() const {
        return fixingDays_;
    }

    inline Spread FloatingRateCoupon::spread() const {
        return spread_;
    }

    inline Rate FloatingRateCoupon::convexityAdjustment(Rate) const {
        return 0.0;
    }

    inline void FloatingRateCoupon::accept(AcyclicVisitor& v) {
        Visitor<FloatingRateCoupon>* v1 =
            dynamic_cast<Visitor<FloatingRateCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            Coupon::accept(v);
    }

}


#endif
