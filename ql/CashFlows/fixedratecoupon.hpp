
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate

    \fullpath
    ql/CashFlows/%fixedratecoupon.hpp
*/

// $Id$

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include "ql/CashFlows/coupon.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! %coupon paying a fixed interest rate
        class FixedRateCoupon : public Coupon {
          public:
            FixedRateCoupon(double nominal, Rate rate, 
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention,
                const Handle<DayCounter>& dayCounter,
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
