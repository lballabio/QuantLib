
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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

    }

}


#endif
