
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
/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate

    \fullpath
    ql/CashFlows/%fixedratecoupon.hpp
*/

// $Id$

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include <ql/CashFlows/coupon.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! %coupon paying a fixed interest rate
        class FixedRateCoupon : public Coupon {
          public:
            FixedRateCoupon(double nominal, Rate rate,
                const Calendar& calendar,
                RollingConvention rollingConvention,
                const DayCounter& dayCounter,
                const Date& startDate, const Date& endDate,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date())
            : Coupon(nominal, calendar, rollingConvention, dayCounter,
              startDate, endDate, refPeriodStart, refPeriodEnd),
              rate_(rate) {}
            //! \name CashFlow interface
            //@{
            double amount() const;
            //@}
            //! \name Coupon interface
            //@{
            double accruedAmount(const Date&) const;
            //@}
            //! \name Inspectors
            //@{
            Rate rate() const;
            //@}
          private:
            Rate rate_;
        };


        // inline definitions

        inline double FixedRateCoupon::amount() const {
            return nominal()*rate_*accrualPeriod();
        }

        inline Rate FixedRateCoupon::rate() const {
            return rate_;
        }

        inline double FixedRateCoupon::accruedAmount(const Date& d) const {
            if (d <= startDate_ || d >= endDate_) {
                return 0.0;
            } else {
                return nominal()*rate_*dayCounter_.yearFraction(startDate_,d,
                    refPeriodStart_,refPeriodEnd_);
            }
        }
        
    }

}


#endif
