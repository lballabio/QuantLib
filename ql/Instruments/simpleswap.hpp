
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
            double nominal() const { 
                const CashFlows::FixedRateCoupon* coupon =
                #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                    fixedLeg()[0].downcast<CashFlows::FixedRateCoupon>();
                #else
                    dynamic_cast<const CashFlows::FixedRateCoupon*>
                        (fixedLeg()[0].pointer());
                #endif
                QL_ENSURE(coupon != 0, "not a fixed-rate coupon");
                return coupon->nominal(); 
            }
            Rate fixedRate() const { 
                const CashFlows::FixedRateCoupon* coupon =
                #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                    fixedLeg()[0].downcast<CashFlows::FixedRateCoupon>();
                #else
                    dynamic_cast<const CashFlows::FixedRateCoupon*>
                        (fixedLeg()[0].pointer());
                #endif
                QL_ENSURE(coupon != 0, "not a fixed-rate coupon");
                return coupon->rate(); 
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
