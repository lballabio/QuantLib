
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file simpleswap.cpp
    \brief Simple fixed-rate vs Libor swap

    \fullpath
    ql/Instruments/%simpleswap.cpp
*/

// $Id$

#include <ql/Instruments/simpleswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>

namespace QuantLib {

    using CashFlows::FixedRateCouponVector;
    using CashFlows::FloatingRateCouponVector;
    using Indexes::Xibor;

    namespace Instruments {

        SimpleSwap::SimpleSwap(bool payFixedRate,
          const Date& startDate, int n, TimeUnit units,
          const Calendar& calendar,
          RollingConvention rollingConvention,
          double nominal,
          int fixedFrequency,
          Rate fixedRate,
          bool fixedIsAdjusted,
          const DayCounter& fixedDayCount,
          int floatingFrequency,
          const Handle<Xibor>& index,
          int indexFixingDays,
          Spread spread,
          const RelinkableHandle<TermStructure>& termStructure,
          const std::string& isinCode, const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(),
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description),
          payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
          nominal_(nominal) {
                        
            maturity_ = calendar.roll(startDate.plus(n,units),rollingConvention);
            
            std::vector<Handle<CashFlow> >::const_iterator i;
            if (payFixedRate_) {
                firstLeg_ = FixedRateCouponVector(
                    std::vector<double>(1,nominal), 
                    std::vector<Rate>(1,fixedRate), startDate, maturity_, 
                    fixedFrequency, calendar, rollingConvention, 
                    fixedIsAdjusted, fixedDayCount, fixedDayCount);
                secondLeg_ = FloatingRateCouponVector(
                    std::vector<double>(1,nominal), startDate, maturity_, 
                    floatingFrequency, calendar, rollingConvention, 
                    index, indexFixingDays, std::vector<Spread>(1,spread));
                for (i = secondLeg_.begin(); i < secondLeg_.end(); ++i)
                    registerWith(*i);
            } else {
                // I know I'm duplicating the initializations, but the 
                // alternative is to duplicate data
                firstLeg_ = FloatingRateCouponVector(
                    std::vector<double>(1,nominal), startDate, maturity_, 
                    floatingFrequency, calendar, rollingConvention, 
                    index, indexFixingDays, std::vector<Spread>(1,spread));
                secondLeg_ = FixedRateCouponVector(
                    std::vector<double>(1,nominal), 
                    std::vector<Rate>(1,fixedRate), startDate, maturity_, 
                    fixedFrequency, calendar, rollingConvention, 
                    fixedIsAdjusted, fixedDayCount, fixedDayCount);
                for (i = firstLeg_.begin(); i < firstLeg_.end(); ++i)
                    registerWith(*i);
            }
        }

    }

}

