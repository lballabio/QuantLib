
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

/*! \file swaption.cpp
    \brief EuropeanSwaption

    \fullpath
    ql/Instruments/%swaption.cpp
*/

// $Id$

#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/CashFlows/fixedratecoupon.hpp"
#include "ql/Instruments/europeanswaption.hpp"
#include "ql/Math/normaldistribution.hpp"
#include "ql/Pricers/couponbondoption.hpp"

namespace QuantLib {

    namespace Instruments {

        using CashFlows::FixedRateCoupon;
        using CashFlows::FloatingRateCoupon;
        using Pricers::CouponBondOption;

        EuropeanSwaption::EuropeanSwaption( const Handle<SimpleSwap>& swap,
            Date maturity,
            RelinkableHandle<TermStructure> termStructure) 
        : swap_(swap), maturity_(maturity), termStructure_(termStructure),
          endTimes_(0), coupons_(0) {

            NPV_ = 0.0;
            Date today = termStructure_->minDate();
            DayCounter counter = termStructure_->dayCounter();
            const std::vector<Handle<CashFlow> >& fixedLeg = swap_->fixedLeg();
            for (unsigned i=0; i<fixedLeg.size(); i++) {
                endTimes_.push_back(counter.yearFraction(today, 
                    fixedLeg[i]->date()));
                coupons_.push_back(fixedLeg[i]->amount());
            }
            coupons_.back() += swap_->nominal();
        }

        void EuropeanSwaption::performCalculations() const {
            QL_REQUIRE(!model_.isNull(), "Cannot price without model!");

            Option::Type type;
            if (swap_->payFixedRate())
                type = Option::Put;
            else
                type = Option::Call;
            Date settlement = termStructure_->settlementDate();
            Time maturity = termStructure_->dayCounter().yearFraction(settlement, maturity_);
            CouponBondOption cbo(maturity, endTimes_, coupons_, type, 
                swap_->nominal(), model_);
            NPV_ = cbo.value(termStructure_->forward(0.0));
        }

    }

}
