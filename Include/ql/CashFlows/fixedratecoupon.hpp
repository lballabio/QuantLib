
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.2  2001/05/31 13:17:36  lballabio
//  Added SimpleSwap
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include "ql/CashFlows/accruingcoupon.hpp"
#include "ql/rate.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Coupon paying a fixed annual rate
        class FixedRateCoupon : public AccruingCoupon {
          public:
            FixedRateCoupon(double nominal, Rate rate, 
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention,
                const Handle<DayCounter>& dayCounter,
                const Date& startDate, const Date& endDate,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date())
            : AccruingCoupon(calendar, rollingConvention, dayCounter, 
              startDate, endDate, refPeriodStart, refPeriodEnd), 
              nominal_(nominal), rate_(rate) {}
            // CashFlow interface
            double amount() const;
            // other inspectors
            double nominal() const { return nominal_; }
          private:
            double nominal_;
            Rate rate_;
        };

        inline double FixedRateCoupon::amount() const {
            return nominal_*rate_*accrualPeriod();
        }
        
    }

}


#endif
