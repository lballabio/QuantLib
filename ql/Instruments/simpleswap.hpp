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
/*! \file simpleswap.hpp
    \brief Simple fixed-rate vs Libor swap

    \fullpath
    ql/Instruments/%simpleswap.hpp
*/

// $Id$

#ifndef quantlib_simple_swap_h
#define quantlib_simple_swap_h

#include <ql/Instruments/swap.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>

namespace QuantLib {

    namespace Instruments {

        //! Simple fixed-rate vs Libor swap
        class SimpleSwap : public Swap {
          public:
            SimpleSwap(
                bool payFixedRate,
                const std::vector<Handle<CashFlow> >& fixedLeg,
                const std::vector<Handle<CashFlow> >& floatingLeg,
                // hook to term structure
                const RelinkableHandle<TermStructure>& termStructure,
                // description
                const std::string& isinCode = "",
                const std::string& description = "") 
            : Swap(std::vector<Handle<CashFlow> >(),
                   std::vector<Handle<CashFlow> >(),
                   termStructure, isinCode, description),
              payFixedRate_(payFixedRate) {
                if (payFixedRate) {
                    firstLeg_ = fixedLeg;
                    secondLeg_ = floatingLeg;
                } else {
                    firstLeg_ = floatingLeg;
                    secondLeg_ = fixedLeg;
                }
            }

            SimpleSwap(bool payFixedRate,
                // dates
                const Date& startDate, int n, TimeUnit units,
                const Calendar& calendar,
                RollingConvention rollingConvention,
                /* nominals (if the vector length is lower than the number
                   of coupons, the last nominal will prevail for the
                   remaining coupons)
                */
                const std::vector<double>& nominals,
                // fixed leg
                int fixedFrequency,
                /* fixed coupon rates (if the vector length is lower than
                   the number of coupons, the last rate will prevail for
                   the remaining coupons)
                */
                const std::vector<Rate>& couponRates,
                bool fixedIsAdjusted,
                const DayCounter& fixedDayCount,
                // floating leg
                int floatingFrequency,
                const Handle<Indexes::Xibor>& index,
                int indexFixingDays,
                const std::vector<Spread>& spreads,
                // hook to term structure
                const RelinkableHandle<TermStructure>& termStructure,
                // description
                const std::string& isinCode = "",
                const std::string& description = "");
            double fixedLegBPS() const;
            double floatingLegBPS() const;
            const Date& maturity() const { return maturity_; }
            //added by Sad
            const std::vector<Handle<CashFlow> >& fixedLeg() const {
                if (payFixedRate_) {
                    return firstLeg_;
                } else {
                    return secondLeg_;
                } 
            }   
            const std::vector<Handle<CashFlow> >& floatingLeg() const {
                if (payFixedRate_) {
                    return secondLeg_;
                } else {
                    return firstLeg_;
                } 
            }
            bool payFixedRate() const {
                return payFixedRate_;
            }
          private:
            bool payFixedRate_;
            Date maturity_;
        };


        // inline definitions

        inline double SimpleSwap::fixedLegBPS() const {
            return (payFixedRate_ ? firstLegBPS() : secondLegBPS());
        }

        inline double SimpleSwap::floatingLegBPS() const {
            return (payFixedRate_ ? secondLegBPS() : firstLegBPS());
        }

    }

}


#endif
