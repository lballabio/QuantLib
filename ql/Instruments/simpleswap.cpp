
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
*/

#include <ql/Instruments/simpleswap.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>

namespace QuantLib {

    using CashFlows::FixedRateCouponVector;
    using CashFlows::FloatingRateCouponVector;
    using CashFlows::Coupon;
    using Indexes::Xibor;

    namespace Instruments {

        SimpleSwap::SimpleSwap(
              bool payFixedRate,
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
              const std::string& isinCode, 
              const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(),
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description),
          payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
          nominal_(nominal) {
                        
            maturity_ = calendar.roll(startDate.plus(n,units),
                                      rollingConvention);

            Scheduler fixedScheduler = 
                MakeScheduler(calendar,startDate,maturity_,
                              fixedFrequency,rollingConvention,
                              fixedIsAdjusted);
            Scheduler floatScheduler =
                MakeScheduler(calendar,startDate,maturity_,
                              floatingFrequency,rollingConvention,
                              true);
            
            std::vector<Handle<CashFlow> > fixedLeg =
                FixedRateCouponVector(std::vector<double>(1,nominal), 
                                      std::vector<Rate>(1,fixedRate), 
                                      fixedDayCount,fixedDayCount,
                                      fixedScheduler);
            std::vector<Handle<CashFlow> > floatingLeg =
                FloatingRateCouponVector(std::vector<double>(1,nominal),
                                         index, indexFixingDays, 
                                         std::vector<Spread>(1,spread),
                                         floatScheduler);
            std::vector<Handle<CashFlow> >::const_iterator i;
            for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
                registerWith(*i);

            if (payFixedRate_) {
                firstLeg_ = fixedLeg;
                secondLeg_ = floatingLeg;
            } else {
                firstLeg_ = floatingLeg;
                secondLeg_ = fixedLeg;
            }
        }

        SimpleSwap::SimpleSwap(
                bool payFixedRate,
                const Date& maturity,
                double nominal,
                Rate fixedRate,
                const DayCounter& fixedDayCount,
                const Handle<Xibor>& index,
                int indexFixingDays,
                Spread spread,
                const RelinkableHandle<TermStructure>& termStructure,
                Scheduler& fixedScheduler, Scheduler& floatScheduler,
                const std::string& isinCode, const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(),
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description),
          payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
          nominal_(nominal), maturity_(maturity) {

            std::vector<Handle<CashFlow> > fixedLeg =
                FixedRateCouponVector(std::vector<double>(1,nominal), 
                                      std::vector<Rate>(1,fixedRate), 
                                      fixedDayCount,fixedDayCount,
                                      fixedScheduler);
            std::vector<Handle<CashFlow> > floatingLeg =
                FloatingRateCouponVector(std::vector<double>(1,nominal),
                                         index, indexFixingDays, 
                                         std::vector<Spread>(1,spread),
                                         floatScheduler);
            std::vector<Handle<CashFlow> >::const_iterator i;
            for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
                registerWith(*i);
            maturity_ = floatingLeg.back()->date();

            if (payFixedRate_) {
                firstLeg_ = fixedLeg;
                secondLeg_ = floatingLeg;
            } else {
                firstLeg_ = floatingLeg;
                secondLeg_ = fixedLeg;
            }
        }
       
        SimpleSwap::SimpleSwap(
                bool payFixedRate,
                const Date& startDate, const Date& maturity,
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
                const Date& fixedStubDate, bool fixedFromEnd, 
                bool fixedLongFinal,
                const Date& floatStubDate, bool floatFromEnd, 
                bool floatLongFinal,
                const std::string& isinCode, const std::string& description)
        : Swap(std::vector<Handle<CashFlow> >(),
               std::vector<Handle<CashFlow> >(),
               termStructure, isinCode, description),
          payFixedRate_(payFixedRate), fixedRate_(fixedRate), spread_(spread), 
          nominal_(nominal), maturity_(maturity) {

            Scheduler fixedScheduler = 
                MakeScheduler(calendar,startDate,maturity,
                              fixedFrequency,rollingConvention,
                              fixedIsAdjusted).
                    withStubDate(fixedStubDate).
                    backwards(fixedFromEnd).
                    longFinalPeriod(fixedLongFinal);
            Scheduler floatScheduler =
                MakeScheduler(calendar,startDate,maturity,
			          floatingFrequency,rollingConvention,
			          true).
                withStubDate(floatStubDate).
                backwards(floatFromEnd).
                longFinalPeriod(floatLongFinal);

            std::vector<Handle<CashFlow> > fixedLeg =
                FixedRateCouponVector(std::vector<double>(1,nominal), 
                                      std::vector<Rate>(1,fixedRate), 
                                      fixedDayCount,fixedDayCount,
                                      fixedScheduler);
            std::vector<Handle<CashFlow> > floatingLeg =
                FloatingRateCouponVector(std::vector<double>(1,nominal),
                                         index, indexFixingDays, 
                                         std::vector<Spread>(1,spread),
                                         floatScheduler);
            std::vector<Handle<CashFlow> >::const_iterator i;
            for (i = floatingLeg.begin(); i < floatingLeg.end(); ++i)
                registerWith(*i);

            if (payFixedRate_) {
                firstLeg_ = fixedLeg;
                secondLeg_ = floatingLeg;
            } else {
                firstLeg_ = floatingLeg;
                secondLeg_ = fixedLeg;
            }
        }
       
    }

}

