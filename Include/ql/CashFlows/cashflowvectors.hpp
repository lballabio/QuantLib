
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

/*! \file cashflowvectors.hpp
    \brief Cash flow vector builders

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.2  2001/06/01 16:50:16  lballabio
//  Term structure on deposits and swaps
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_cash_flow_vectors_hpp
#define quantlib_cash_flow_vectors_hpp

#include "ql/cashflow.hpp"
#include "ql/CashFlows/fixedratecoupon.hpp"
#include "ql/CashFlows/indexlinkedcoupon.hpp"
#include "ql/CashFlows/parcoupon.hpp"
#include <vector>

namespace QuantLib {

    namespace CashFlows {
        
        class FixedRateCouponVector : public std::vector<Handle<CashFlow> > {
          public:
            FixedRateCouponVector(
                const std::vector<double>& nominals, 
                const std::vector<Rate>& couponRates, 
                const Date& startDate, const Date& endDate, 
                int frequency, const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, bool isAdjusted, 
                const Handle<DayCounter>& dayCount, 
                const Date& stubDate = Date(), 
                const Handle<DayCounter>& firstPeriodDayCount = 
                    Handle<DayCounter>());
        };

        class IndexLinkedCouponVector : public std::vector<Handle<CashFlow> > {
          public:
            IndexLinkedCouponVector(
                const std::vector<double>& nominals, 
                const Handle<Index>& index, int fixingDays,
                const std::vector<Spread>& spreads, 
                const Date& startDate, const Date& endDate, 
                int frequency, const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, bool isAdjusted, 
                const Handle<DayCounter>& dayCount, 
                const Date& stubDate = Date(), 
                const Handle<DayCounter>& firstPeriodDayCount =
                    Handle<DayCounter>());
        };

        class ParCouponVector : public std::vector<Handle<CashFlow> > {
          public:
            ParCouponVector(
                const std::vector<double>& nominals, 
                const Indexes::Xibor& index, 
                const std::vector<Spread>& spreads, 
                const Date& startDate, const Date& endDate, 
                int frequency, const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, 
                const Handle<DayCounter>& dayCount, 
                const RelinkableHandle<TermStructure>& termStructure,
                const Date& stubDate = Date(), 
                const Handle<DayCounter>& firstPeriodDayCount =
                    Handle<DayCounter>());
        };

    }

}


#endif
