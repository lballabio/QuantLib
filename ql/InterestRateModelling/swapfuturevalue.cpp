
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

/*! \file swapfuturevalue.cpp
    \brief Abstract interest rate model class

    \fullpath
    ql/InterestRateModelling/%swapfuturevalue.cpp
*/

// $Id$

#include "ql/InterestRateModelling/swapfuturevalue.hpp"

namespace QuantLib {
    using Instruments::SimpleSwap;
    using CashFlows::Coupon;

    namespace InterestRateModelling {
        double swapFutureValue(const Handle<SimpleSwap>& swap, 
            const Handle<Model>& model, Rate rate, Time time) {

            Date settlement = model->termStructure()->settlementDate();
            double fixedLegValue = 0.0;
            std::vector<Handle<CashFlow> > fixedLeg = swap->fixedLeg();

            unsigned int i;
            for (i=0; i<fixedLeg.size(); i++) {
                Date cashFlowDate = fixedLeg[i]->date();
                Time t = model->termStructure()->dayCounter().
                    yearFraction(settlement, cashFlowDate);
                if (t >= time) {
                    fixedLegValue += fixedLeg[i]->amount() *
                        model->discountBond(time, t, rate);
                }
            }
            double floatingLegValue = 0.0;
            std::vector<Handle<CashFlow> > floatingLeg = swap->floatingLeg();

            for (i=0; i<floatingLeg.size(); i++) {
                Date cashFlowDate = floatingLeg[i]->date();
                Time t = model->termStructure()->dayCounter().
                    yearFraction(settlement, cashFlowDate);
                if (t >= time) {
                    Handle<Coupon> coupon = floatingLeg[i];
                    QL_ENSURE(!coupon.isNull(), "not a coupon");
                    Time startTime = 
                        model->termStructure()->dayCounter().yearFraction(
                            settlement, coupon->accrualStartDate());
                    Time endTime = model->termStructure()->dayCounter().
                        yearFraction(settlement,  coupon->accrualEndDate());
                    double sd = model->discountBond(time, startTime, rate);
                    double ed = model->discountBond(time, endTime, rate);
                    floatingLegValue += (sd/ed - 1.0)*
                        coupon->nominal()*model->discountBond(time, t, rate); 
                }
            }
            if (swap->payFixedRate())
                return (floatingLegValue - fixedLegValue);
            else 
                return (fixedLegValue - floatingLegValue);
        }
    }
}

