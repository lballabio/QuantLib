
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
/*! \file simpleswap.cpp
    \brief Simple fixed-rate vs Libor swap

    \fullpath
    ql/Instruments/%simpleswap.cpp
*/

// $Id$

#include <ql/Instruments/simpleswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>

namespace QuantLib {

    using CashFlows::FixedRateCoupon;
    using CashFlows::FixedRateCouponVector;
    using CashFlows::FloatingRateCouponVector;
    using Indexes::Xibor;

    namespace Instruments {

        SimpleSwap::SimpleSwap(bool payFixedRate,
          const Date& startDate, int n, TimeUnit units,
          const Calendar& calendar,
          RollingConvention rollingConvention,
          const std::vector<double>& nominals,
          int fixedFrequency,
          const std::vector<Rate>& couponRates,
          bool fixedIsAdjusted,
          const DayCounter& fixedDayCount,
          int floatingFrequency,
          const Handle<Xibor>& index,
          int indexFixingDays,
          const std::vector<Spread>& spreads,
          const RelinkableHandle<TermStructure>& termStructure,
          const std::string& isinCode, const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(),
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description),
          payFixedRate_(payFixedRate) {
            maturity_ = calendar.advance(
                startDate,n,units,rollingConvention);
            if (payFixedRate_) {
                firstLeg_ = FixedRateCouponVector(nominals,
                    couponRates, startDate, maturity_,
                    fixedFrequency, calendar, rollingConvention,
                    fixedIsAdjusted, fixedDayCount, fixedDayCount);
                secondLeg_ = FloatingRateCouponVector(nominals,
                    startDate, maturity_, floatingFrequency,
                    calendar, rollingConvention, termStructure,
                    index, indexFixingDays, spreads);
            } else {
                firstLeg_ = FloatingRateCouponVector(nominals,
                    startDate, maturity_, floatingFrequency,
                    calendar, rollingConvention, termStructure,
                    index, indexFixingDays, spreads);
                secondLeg_ = FixedRateCouponVector(nominals,
                    couponRates, startDate, maturity_,
                    fixedFrequency, calendar, rollingConvention,
                    fixedIsAdjusted, fixedDayCount, fixedDayCount);
            }
            // we should register as observer with the cash flows. However,
            // the base Swap class already registers as observer with
            // the term structure, which is also the same passed to floating
            // rate coupons; the index is only used for past fixings; and
            // fixed rate coupons are not modifiable.
        }

    }

}

