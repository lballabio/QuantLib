
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
/*! \file swap.cpp
    \brief Interest rate swap

    \fullpath
    ql/Instruments/%swap.cpp
*/

// $Id$

#include <ql/Instruments/swap.hpp>
#include <ql/CashFlows/coupon.hpp>

namespace QuantLib {

    using CashFlows::Coupon;

    namespace Instruments {

        Swap::Swap(const std::vector<Handle<CashFlow> >& firstLeg,
            const std::vector<Handle<CashFlow> >& secondLeg,
            const RelinkableHandle<TermStructure>& termStructure,
            const std::string& isinCode, const std::string& description)
        : Instrument(isinCode,description), firstLeg_(firstLeg),
          secondLeg_(secondLeg), termStructure_(termStructure) {
            termStructure_.registerObserver(this);
            std::vector<Handle<CashFlow> >::iterator i;
            for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
                (*i)->registerObserver(this);
            for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
                (*i)->registerObserver(this);
        }

        Swap::~Swap() {
            termStructure_.unregisterObserver(this);
            std::vector<Handle<CashFlow> >::iterator i;
            for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
                (*i)->unregisterObserver(this);
            for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
                (*i)->unregisterObserver(this);
        }

        void Swap::performCalculations() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "Swap::performCalculations trying to price swap "
                "on null term structure");
            Date settlement = termStructure_->settlementDate();
            NPV_ = 0.0;
            firstLegBPS_ = 0.0;
            secondLegBPS_ = 0.0;
            isExpired_ = true;
            // subtract first leg cash flows and BPS
            for (Size i=0; i<firstLeg_.size(); i++) {
                Date cashFlowDate = firstLeg_[i]->date();
                if (cashFlowDate >= settlement) {
                    isExpired_ = false;  // keeping track of whether this
                                         // was already set isn't worth the
                                         // effort
                    NPV_ -= firstLeg_[i]->amount() *
                        termStructure_->discount(cashFlowDate);
                    Handle<Coupon> coupon = firstLeg_[i];
                    // check that the downcast succeeded
                    // and subtract coupon sensitivity
                    if (!coupon.isNull()) {
                        firstLegBPS_ -= coupon->accrualPeriod() *
                            coupon->nominal() *
                            termStructure_->discount(coupon->date());
                    }
                }
            }
            // add second leg cash flows and BPS
            for (Size j=0; j<secondLeg_.size(); j++) {
                Date cashFlowDate = secondLeg_[j]->date();
                if (cashFlowDate >= settlement) {
                    isExpired_ = false;
                    NPV_ += secondLeg_[j]->amount() *
                        termStructure_->discount(cashFlowDate);
                    Handle<Coupon> coupon = secondLeg_[j];
                    // check that the downcast succeeded
                    // and add coupon sensitivity
                    if (!coupon.isNull()) {
                        secondLegBPS_ += coupon->accrualPeriod() *
                            coupon->nominal() *
                            termStructure_->discount(coupon->date());
                    }
                }
            }
        }

    }

}
