
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

/*! \file accruingcoupon.hpp
    \brief Coupon accruing over a fixed period

    \fullpath
    ql/CashFlows/%accruingcoupon.hpp
*/

// $Id$

#ifndef quantlib_accruing_coupon_hpp
#define quantlib_accruing_coupon_hpp

#include "ql/cashflow.hpp"
#include "ql/calendar.hpp"
#include "ql/daycounter.hpp"

namespace QuantLib {

    namespace CashFlows {

        /*! \class AccruingCoupon ql/CashFlows/accruingcoupon.hpp
            \brief coupon accruing over a fixed period

            This class implements part of the CashFlow interface but it
            is still abstract and provides derived classes with methods for
            accrual period calculations.
        */
        class AccruingCoupon : public CashFlow {
          public:
            AccruingCoupon(const Handle<Calendar>& calendar,
                RollingConvention rollingConvention,
                const Handle<DayCounter>& dayCounter,
                const Date& startDate, const Date& endDate,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date()) :
                startDate_(startDate),
                endDate_(endDate),
                refPeriodStart_(refPeriodStart),
                refPeriodEnd_(refPeriodEnd),
                calendar_(calendar),
                rollingConvention_(rollingConvention),
                dayCounter_(dayCounter)
                {}
            //! \name Partial CashFlow interface
            //@{
            Date date() const;
            //@}
            //! \name Inspectors
            //@{
            //! start of the accrual period
            const Date& accrualStartDate() const { return startDate_; }
            //! end of the accrual period
            const Date& accrualEndDate() const { return endDate_; }
            //! accrual period as fraction of year
            double accrualPeriod() const;
            //! accrual period in days
            int accrualDays() const;
            //@}
          protected:
            Date startDate_, endDate_, refPeriodStart_, refPeriodEnd_;
            Handle<Calendar> calendar_;
            RollingConvention rollingConvention_;
            Handle<DayCounter> dayCounter_;
        };

        inline Date AccruingCoupon::date() const {
            return calendar_->roll(endDate_,rollingConvention_);
        }

        inline double AccruingCoupon::accrualPeriod() const {
            return dayCounter_->yearFraction(startDate_,endDate_,
                refPeriodStart_,refPeriodEnd_);
        }

        inline int AccruingCoupon::accrualDays() const {
            return dayCounter_->dayCount(startDate_,endDate_);
        }

    }

}


#endif
