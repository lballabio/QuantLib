
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

/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate
*/

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include <ql/CashFlows/coupon.hpp>

namespace QuantLib {

    //! %Coupon paying a fixed interest rate
    class FixedRateCoupon : public Coupon {
      public:
        FixedRateCoupon(double nominal, 
                        const Date& paymentDate,
                        Rate rate,
                        const DayCounter& dayCounter,
                        const Date& startDate, const Date& endDate,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date())
        : Coupon(nominal, paymentDate, startDate, endDate, 
                 refPeriodStart, refPeriodEnd),
          rate_(rate), dayCounter_(dayCounter) {}
        //! \name CashFlow interface
        //@{
        double amount() const;
        //@}
        //! \name Coupon interface
        //@{
        DayCounter dayCounter() const;
        double accruedAmount(const Date&) const;
        //@}
        //! \name Inspectors
        //@{
        Rate rate() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Rate rate_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline double FixedRateCoupon::amount() const {
        return nominal()*rate_*accrualPeriod();
    }

    inline Rate FixedRateCoupon::rate() const {
        return rate_;
    }

    inline DayCounter FixedRateCoupon::dayCounter() const {
        return dayCounter_;
    }

    inline double FixedRateCoupon::accruedAmount(const Date& d) const {
        if (d <= accrualStartDate_ || d > paymentDate_) {
            return 0.0;
        } else {
            return nominal()*rate_*
                dayCounter_.yearFraction(accrualStartDate_,
                                         QL_MIN(d,accrualEndDate_),
                                         refPeriodStart_,
                                         refPeriodEnd_);
        }
    }

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
