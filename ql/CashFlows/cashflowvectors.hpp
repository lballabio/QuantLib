
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

/*! \file cashflowvectors.hpp
    \brief Cash flow vector builders

    \fullpath
    ql/CashFlows/%cashflowvectors.hpp
*/

// $Id$

#ifndef quantlib_cash_flow_vectors_hpp
#define quantlib_cash_flow_vectors_hpp

#include "ql/cashflow.hpp"
#include "ql/Indexes/xibor.hpp"

namespace QuantLib {

    namespace CashFlows {

        //! helper class building a sequence of fixed rate coupons
        class FixedRateCouponVector
        : public std::vector<Handle<CashFlow> > {
          public:
            FixedRateCouponVector(
                const std::vector<double>& nominals,
                const std::vector<Rate>& couponRates,
                const Date& startDate, const Date& endDate,
                int frequency, const Calendar& calendar,
                RollingConvention rollingConvention, bool isAdjusted,
                const DayCounter& dayCount,
                const DayCounter& firstPeriodDayCount,
                const Date& stubDate = Date());
        };

        //! helper class building a sequence of floating rate coupons
        /*! \warning The passing of a non-null stub date - i.e., the creation
            of a short/long first coupon - is currently disabled.
            \todo A suitable algorithm should be implemented for the
            calculation of the interpolated index fixing for a
            short/long first coupon.
        */
        class FloatingRateCouponVector
        : public std::vector<Handle<CashFlow> > {
          public:
            FloatingRateCouponVector(
                const std::vector<double>& nominals,
                const Date& startDate, const Date& endDate,
                int frequency, const Calendar& calendar,
                RollingConvention rollingConvention,
                const RelinkableHandle<TermStructure>& termStructure,
                const Handle<Indexes::Xibor>& index,
                const std::vector<Spread>& spreads = std::vector<Spread>(),
                const Date& stubDate = Date());
        };

    }

}


#endif
