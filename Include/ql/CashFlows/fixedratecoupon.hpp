
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
    \fullpath Include/ql/CashFlows/%fixedratecoupon.hpp
    \brief Coupon paying a fixed annual rate

*/

// $Id$
//  $Log$
//  Revision 1.9  2001/08/09 14:59:46  sigmud
//  header modification
//
//  Revision 1.8  2001/08/08 11:07:48  sigmud
//  inserting \fullpath for doxygen
//
//  Revision 1.7  2001/08/07 11:25:53  sigmud
//  copyright header maintenance
//
//  Revision 1.6  2001/07/25 15:47:27  sigmud
//  Change from quantlib.sourceforge.net to quantlib.org
//
//  Revision 1.5  2001/07/24 08:49:32  sigmud
//  pruned redundant header inclusions
//
//  Revision 1.4  2001/07/05 15:57:22  lballabio
//  Collected typedefs in a single file
//
//  Revision 1.3  2001/06/15 13:52:07  lballabio
//  Reworked indexes
//
//  Revision 1.2  2001/05/31 13:17:36  lballabio
//  Added SimpleSwap
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_fixed_rate_coupon_hpp
#define quantlib_fixed_rate_coupon_hpp

#include "ql/CashFlows/accruingcoupon.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Coupon paying a fixed interest rate
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
            //! \name CashFlow interface
            //@{
            double amount() const;
            //@}
            //! \name Inspectors
            //@{
            double nominal() const { return nominal_; }
            Rate rate() const { return rate_; }
            //@}
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
