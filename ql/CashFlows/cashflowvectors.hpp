

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
/*! \file cashflowvectors.hpp
    \brief Cash flow vector builders

    \fullpath
    ql/CashFlows/%cashflowvectors.hpp
*/

// $Id$

#ifndef quantlib_cash_flow_vectors_hpp
#define quantlib_cash_flow_vectors_hpp

#include <ql/cashflow.hpp>
#include <ql/Indexes/xibor.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! helper function building a sequence of fixed rate coupons
        std::vector<Handle<CashFlow> > FixedRateCouponVector(
            const std::vector<double>& nominals,
            const std::vector<Rate>& couponRates,
            const Date& startDate, const Date& endDate,
            int frequency, const Calendar& calendar,
            RollingConvention rollingConvention, bool isAdjusted,
            const DayCounter& dayCount,
            const DayCounter& firstPeriodDayCount,
            const Date& stubDate = Date());

        //! helper function building a sequence of floating rate coupons
        /*! \warning The passing of a non-null stub date - i.e., the creation
            of a short/long first coupon - is currently disabled.
            \todo A suitable algorithm should be implemented for the
            calculation of the interpolated index fixing for a
            short/long first coupon.
        */
        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
            const std::vector<double>& nominals,
            const Date& startDate, const Date& endDate,
            int frequency, const Calendar& calendar,
            RollingConvention rollingConvention,
            const RelinkableHandle<TermStructure>& termStructure,
            const Handle<Indexes::Xibor>& index,
            int fixingDays,
            const std::vector<Spread>& spreads = std::vector<Spread>(),
            const Date& stubDate = Date());

    }

}


#endif
