
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/swap.hpp>
#include <ql/CashFlows/basispointsensitivity.hpp>

namespace QuantLib {

    Swap::Swap(const std::vector<boost::shared_ptr<CashFlow> >& firstLeg,
               const std::vector<boost::shared_ptr<CashFlow> >& secondLeg,
               const RelinkableHandle<TermStructure>& termStructure)
    : firstLeg_(firstLeg), secondLeg_(secondLeg), 
      termStructure_(termStructure) {
        registerWith(termStructure_);
        std::vector<boost::shared_ptr<CashFlow> >::iterator i;
        for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
            registerWith(*i);
        for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
            registerWith(*i);
    }

    bool Swap::isExpired() const {
        Date lastPayment = Date::minDate();
        std::vector<boost::shared_ptr<CashFlow> >::const_iterator i;
        for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
            lastPayment = QL_MAX(lastPayment, (*i)->date());
        for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
            lastPayment = QL_MAX(lastPayment, (*i)->date());
        #if QL_TODAYS_PAYMENTS
        return lastPayment < termStructure_->referenceDate();
        #else
        return lastPayment <= termStructure_->referenceDate();
        #endif
    }

    void Swap::setupExpired() const {
        NPV_ = firstLegBPS_= secondLegBPS_ = 0.0;
    }

    void Swap::performCalculations() const {
        QL_REQUIRE(!termStructure_.isNull(),
                   "Swap::performCalculations trying to price swap "
                   "on null term structure");
        Date settlement = termStructure_->referenceDate();
        NPV_ = 0.0;
        double firstLegNPV_ = 0.0;
        double secondLegNPV_ = 0.0;

        // subtract first leg cash flows
        for (Size i=0; i<firstLeg_.size(); i++) {
            Date cashFlowDate = firstLeg_[i]->date();
            #if QL_TODAYS_PAYMENTS
            if (cashFlowDate >= settlement) {
            #else
            if (cashFlowDate > settlement) {
            #endif
                firstLegNPV_ -= firstLeg_[i]->amount() *
                    termStructure_->discount(cashFlowDate);
            }
        }
        firstLegBPS_ = - BasisPointSensitivity(firstLeg_, termStructure_);

        // add second leg cash flows
        for (Size j=0; j<secondLeg_.size(); j++) {
            Date cashFlowDate = secondLeg_[j]->date();
            #if QL_TODAYS_PAYMENTS
            if (cashFlowDate >= settlement) {
            #else
            if (cashFlowDate > settlement) {
            #endif
                secondLegNPV_ += secondLeg_[j]->amount() *
                    termStructure_->discount(cashFlowDate);
            }
        }
        secondLegBPS_ = BasisPointSensitivity(secondLeg_, termStructure_);

        NPV_ = firstLegNPV_ + secondLegNPV_;
    }

    Date Swap::startDate() const {
        Date d = Date::maxDate();
        Size i;
        for (i=0; i<firstLeg_.size(); i++) {
            boost::shared_ptr<Coupon> c = 
                boost::dynamic_pointer_cast<Coupon>(firstLeg_[i]);
            if (c)
                d = QL_MIN(d, c->accrualStartDate());
        }
        for (i=0; i<secondLeg_.size(); i++) {
            boost::shared_ptr<Coupon> c = 
                boost::dynamic_pointer_cast<Coupon>(secondLeg_[i]);
            if (c)
                d = QL_MIN(d, c->accrualStartDate());
        }
        QL_REQUIRE(d != Date::maxDate(),
                   "Swap::startDate : not enough information available");
        return d;
    }

    Date Swap::maturity() const {
        Date d = Date::minDate();
        Size i;
        for (i=0; i<firstLeg_.size(); i++)
            d = QL_MAX(d, firstLeg_[i]->date());
        for (i=0; i<secondLeg_.size(); i++)
            d = QL_MAX(d, secondLeg_[i]->date());
        QL_REQUIRE(d != Date::minDate(),
                   "Swap::maturity : empty swap");
        return d;
    }

    double Swap::firstLegBPS() const {
        calculate();
        return firstLegBPS_;
    }

    double Swap::secondLegBPS() const {
        calculate();
        return secondLegBPS_;
    }

    TimeBasket Swap::sensitivity(int basis) const {
        calculate();
        TimeBasket basket = BasisPointSensitivityBasket(firstLeg_,
                                                        termStructure_,
                                                        basis);
        basket += BasisPointSensitivityBasket(secondLeg_,
                                              termStructure_,
                                              basis);
        return basket;
    }

}
