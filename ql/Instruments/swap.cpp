
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

/*! \file swap.cpp
    \brief Interest rate swap
*/

#include <ql/Instruments/swap.hpp>

/* The following checks whether the user wants coupon payments with
   date corresponding to the evaluation date to be included in the NPV.
*/
#if defined(QL_EARLY_SWAP_PAYMENTS)
    #define QL_INCLUDE_TODAYS_COUPON 0
#elif defined(QL_LATE_SWAP_PAYMENTS)
    #define QL_INCLUDE_TODAYS_COUPON 1
#elif defined(QL_EARLY_PAYMENTS)
    #define QL_INCLUDE_TODAYS_COUPON 0
#elif defined(QL_LATE_PAYMENTS)
    #define QL_INCLUDE_TODAYS_COUPON 1
#else
    #define QL_INCLUDE_TODAYS_COUPON 0
#endif

namespace QuantLib {

    using namespace CashFlows;

    namespace Instruments {

        Swap::Swap(const std::vector<Handle<CashFlow> >& firstLeg,
            const std::vector<Handle<CashFlow> >& secondLeg,
            const RelinkableHandle<TermStructure>& termStructure,
            const std::string& isinCode, const std::string& description)
        : Instrument(isinCode,description), firstLeg_(firstLeg),
          secondLeg_(secondLeg), termStructure_(termStructure) {
            registerWith(termStructure_);
            std::vector<Handle<CashFlow> >::iterator i;
            for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
                registerWith(*i);
            for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
                registerWith(*i);
        }

        bool Swap::isExpired() const {
            Date lastPayment = Date::minDate();
            std::vector<Handle<CashFlow> >::const_iterator i;
            for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
                lastPayment = QL_MAX(lastPayment, (*i)->date());
            for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
                lastPayment = QL_MAX(lastPayment, (*i)->date());
            #if QL_INCLUDE_TODAYS_COUPON
            return lastPayment < termStructure_->referenceDate();
            #else
            return lastPayment <= termStructure_->referenceDate();
            #endif
        }

        void Swap::setupExpired() const {
            NPV_ = firstLegBPS_= secondLegBPS_ = 0.0;
            sensitivity_ = Handle<TimeBasket>();
        }

        void Swap::performCalculations() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "Swap::performCalculations trying to price swap "
                "on null term structure");
            Date settlement = termStructure_->referenceDate();
            NPV_ = 0.0;
            firstLegBPS_ = 0.0;
            secondLegBPS_ = 0.0;
            double firstLegNPV_ = 0.0;
            double secondLegNPV_ = 0.0;

            BPSBasketCalculator basketbps(termStructure_,2);
            // subtract first leg cash flows and BPS
            BPSCalculator bps1(termStructure_);
            for (Size i=0; i<firstLeg_.size(); i++) {
                Date cashFlowDate = firstLeg_[i]->date();
                #if QL_INCLUDE_TODAYS_COUPON
                if (cashFlowDate >= settlement) {
                #else
                if (cashFlowDate > settlement) {
                #endif
                    firstLegNPV_ -= firstLeg_[i]->amount() *
                        termStructure_->discount(cashFlowDate);
                    firstLeg_[i]->accept(bps1);
                    firstLeg_[i]->accept(basketbps);
                }
            }
            firstLegBPS_ = -bps1.result();

            // add second leg cash flows and BPS
            BPSCalculator bps2(termStructure_);
            for (Size j=0; j<secondLeg_.size(); j++) {
                Date cashFlowDate = secondLeg_[j]->date();
                #if QL_INCLUDE_TODAYS_COUPON
                if (cashFlowDate >= settlement) {
                #else
                if (cashFlowDate > settlement) {
                #endif
                    secondLegNPV_ += secondLeg_[j]->amount() *
                        termStructure_->discount(cashFlowDate);
                    secondLeg_[j]->accept(bps2);
                    secondLeg_[j]->accept(basketbps);
                }
            }
            NPV_ = firstLegNPV_ + secondLegNPV_;
            secondLegBPS_ = bps2.result();
            sensitivity_ = basketbps.result();
        }

    }

}
