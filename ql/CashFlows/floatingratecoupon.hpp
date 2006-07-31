/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl

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
        \todo add gearing unit test
    */
    class FloatingRateCoupon : public Coupon {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated use the gearing-enabled constructor instead
        FloatingRateCoupon(Real nominal,
                           const Date& paymentDate,
                           const Date& startDate,
                           const Date& endDate,
                           Integer fixingDays,
                           Spread spread = 0.0,
                           const Date& refPeriodStart = Date(),
                           const Date& refPeriodEnd = Date());
        #endif
        FloatingRateCoupon(const Date& paymentDate,
                           const Real nominal,
                           const Date& startDate,
                           const Date& endDate,
                           const Integer fixingDays,
                           const Real gearing = 1.0,
                           const Spread spread = 0.0,
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
        Integer fixingDays() const { return fixingDays_; }
        //! fixing date
        virtual Date fixingDate() const = 0;
        //! index gearing, i.e. multiplicative coefficient for the index
        virtual Real gearing() const { return gearing_; }
        //! fixing of the underlying index
        virtual Rate indexFixing() const = 0;
        //! convexity adjustment for the given index fixing
        virtual Rate convexityAdjustment(Rate fixing) const { return 0.0; }
        //! spread paid over the fixing of the underlying index
        virtual Spread spread() const { return spread_; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Integer fixingDays_;
        Real gearing_;
        Spread spread_;
    };


    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    //! \deprecated use the gearing-enabled constructor instead
    inline FloatingRateCoupon::FloatingRateCoupon(
                         Real nominal, const Date& paymentDate,
                         const Date& startDate, const Date& endDate,
                         Integer fixingDays, Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      fixingDays_(fixingDays), gearing_(1.0), spread_(spread) {}
    #endif

    inline FloatingRateCoupon::FloatingRateCoupon(
                         const Date& paymentDate, const Real nominal,
                         const Date& startDate, const Date& endDate,
                         const Integer fixingDays,
                         const Real gearing, const Spread spread,
                         const Date& refPeriodStart, const Date& refPeriodEnd)
    : Coupon(nominal, paymentDate,
             startDate, endDate, refPeriodStart, refPeriodEnd),
      fixingDays_(fixingDays), gearing_(gearing), spread_(spread) {}

    inline Rate FloatingRateCoupon::rate() const {
        Rate f = indexFixing();
        return gearing() * (f + convexityAdjustment(f)) + spread();
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
