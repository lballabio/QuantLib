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

/*! \file capfloor.cpp
    \brief European cap and floor class

    \fullpath
    ql/Instruments/%capfloor.cpp
*/

// $Id$

#include "ql/Instruments/capfloor.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"


namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCoupon;

        EuropeanCapFloor::EuropeanCapFloor( Type type, 
          const Handle<SimpleSwap>& swap, 
          std::vector<Rate> exerciseRates,
          RelinkableHandle<TermStructure> termStructure) 
        : type_(type), swap_(swap), exerciseRates_(exerciseRates), 
          termStructure_(termStructure) {
            std::vector<Handle<CashFlow> > floatingLeg = swap_->floatingLeg();
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            begin = floatingLeg.begin();
            end   = floatingLeg.end();
            NPV_ = 0.0;
            Date today = termStructure_->minDate();
            DayCounter counter = termStructure_->dayCounter();
            unsigned int i=0;
            for (; begin != end; ++begin) {
                const FloatingRateCoupon* coupon = 
                    #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                        begin->downcast<FloatingRateCoupon>();
                    #else
                        dynamic_cast<const FloatingRateCoupon*>(begin->pointer());
                    #endif
                QL_ENSURE(coupon != 0, "not a floating rate coupon");
                Date beginDate = coupon->accrualStartDate();
                Date endDate = coupon->date();
                startTimes_.resize(i+1);
                endTimes_.resize(i+1);
                nominals_.resize(i+1);
                tenors_.resize(i+1);
                startTimes_[i] = counter.yearFraction(today, beginDate);
                endTimes_[i] = counter.yearFraction(today, endDate);
                nominals_[i] = coupon->nominal();
                tenors_[i] = counter.yearFraction(beginDate, endDate);
                i++;
            }
            nPeriods_ = i;
        }

        void EuropeanCapFloor::performCalculations() const {
            QL_REQUIRE(!model_.isNull(), "Cannot price without model!");
            Option::Type optionType;
            if (type_==Cap)
                optionType = Option::Put;
            else
                optionType = Option::Call;
            NPV_ = 0.0;
            for (unsigned i=0; i<nPeriods_; i++) {
                Rate exerciseRate;
                if (i<exerciseRates_.size())
                    exerciseRate = exerciseRates_[i];
                else
                    exerciseRate = exerciseRates_.back();
                double optionStrike = 1.0/(1.0+exerciseRate*tenors_[i]);
                double optionValue = model_->discountBondOption(
                  optionType, optionStrike, startTimes_[i], endTimes_[i]);
                double capletValue = nominals_[i]*(1.0+exerciseRate*tenors_[i])*optionValue;
                NPV_ += capletValue;
            }
        }

    }

}
