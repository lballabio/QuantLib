
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

/*! \file cashflowvectors.hpp
    \brief Cash flow vector builders
*/

#ifndef quantlib_cash_flow_vectors_hpp
#define quantlib_cash_flow_vectors_hpp

#include <ql/cashflow.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/scheduler.hpp>

namespace QuantLib {

    namespace CashFlows {

        //! helper function building a sequence of fixed rate coupons
        std::vector<Handle<CashFlow> > FixedRateCouponVector(
            const Schedule& schedule,
            const std::vector<double>& nominals,
            const std::vector<Rate>& couponRates,
            const DayCounter& dayCount, 
            const DayCounter& firstPeriodDayCount = DayCounter());

        //! helper function building a sequence of fixed rate coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FixedRateCouponVector(
            const std::vector<double>& nominals,
            const std::vector<Rate>& couponRates,
            const Date& startDate, const Date& endDate,
            int frequency, const Calendar& calendar,
            RollingConvention rollingConvention, bool isAdjusted,
            const DayCounter& dayCount,
            const DayCounter& firstPeriodDayCount,
            const Date& stubDate = Date());

        //! helper function building a sequence of fixed rate coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FixedRateCouponVector(
            const std::vector<double>& nominals,
            const std::vector<Rate>& couponRates,
            const DayCounter& dayCount, const DayCounter& firstPeriodDayCount,
            const Schedule& schedule);

        //! helper function building a sequence of fixed rate coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FixedRateCouponVector(
            const std::vector<double>& nominals,
            const std::vector<Rate>& couponRates,
            const std::vector<Date>& dates,
            const Calendar& calendar,
            RollingConvention roll,
            const DayCounter& dayCounter);



        //! helper function building a sequence of par coupons
        /*! \warning The passing of a non-null stub date - i.e., the creation
            of a short/long first coupon - is currently disabled.
            \todo A suitable algorithm should be implemented for the
            calculation of the interpolated index fixing for a
            short/long first coupon.
        */
        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
            const Schedule& schedule,
            const std::vector<double>& nominals,
            const Handle<Indexes::Xibor>& index, int fixingDays,
            const std::vector<Spread>& spreads = std::vector<Spread>());

        //! helper function building a sequence of par coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
            const std::vector<double>& nominals,
            const Date& startDate, const Date& endDate,
            int frequency, const Calendar& calendar,
            RollingConvention rollingConvention,
            const Handle<Indexes::Xibor>& index,
            int fixingDays,
            const std::vector<Spread>& spreads, 
            const Date& stubDate = Date());

        //! helper function building a sequence of par coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
            const std::vector<double>& nominals,
            const Handle<Indexes::Xibor>& index, int fixingDays,
            const std::vector<Spread>& spreads,
            const Schedule& schedule);

        //! helper function building a sequence of par coupons
        /*! \deprecated use the version taking a Schedule as first
                        argument instead */
        std::vector<Handle<CashFlow> > FloatingRateCouponVector(
            const std::vector<double>& nominals,
            const std::vector<Spread>& spreads,
            const std::vector<Date>& dates,
            const Handle<Indexes::Xibor>& index, int fixingDays,
            const Calendar& calendar,
            RollingConvention roll);
       
    }

}


#endif
