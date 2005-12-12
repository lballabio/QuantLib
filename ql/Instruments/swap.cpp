/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Instruments/swap.hpp>
#include <ql/CashFlows/basispointsensitivity.hpp>
#include <ql/CashFlows/analysis.hpp>

namespace QuantLib {

    Swap::Swap(const std::vector<boost::shared_ptr<CashFlow> >& firstLeg,
               const std::vector<boost::shared_ptr<CashFlow> >& secondLeg,
               const Handle<YieldTermStructure>& termStructure)
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
        Date settlement = termStructure_->referenceDate();
        std::vector<boost::shared_ptr<CashFlow> >::const_iterator i;
        for (i = firstLeg_.begin(); i!= firstLeg_.end(); ++i)
            if (!(*i)->hasOccurred(settlement))
                return false;
        for (i = secondLeg_.begin(); i!= secondLeg_.end(); ++i)
            if (!(*i)->hasOccurred(settlement))
                return false;
        return true;
    }

    void Swap::setupExpired() const {
        Instrument::setupExpired();
        firstLegBPS_= secondLegBPS_ = 0.0;
    }

    void Swap::performCalculations() const {
        QL_REQUIRE(!termStructure_.empty(), "no term structure set");
        Date settlement = termStructure_->referenceDate();

        NPV_ = - Cashflows::npv(firstLeg_,termStructure_)
               + Cashflows::npv(secondLeg_,termStructure_);
        errorEstimate_ = Null<Real>();

        firstLegBPS_ = - Cashflows::bps(firstLeg_, termStructure_);
        secondLegBPS_ = + Cashflows::bps(secondLeg_, termStructure_);
    }

    Date Swap::startDate() const {
        Date d = Date::maxDate();
        Size i;
        for (i=0; i<firstLeg_.size(); i++) {
            boost::shared_ptr<Coupon> c =
                boost::dynamic_pointer_cast<Coupon>(firstLeg_[i]);
            if (c)
                d = std::min(d, c->accrualStartDate());
        }
        for (i=0; i<secondLeg_.size(); i++) {
            boost::shared_ptr<Coupon> c =
                boost::dynamic_pointer_cast<Coupon>(secondLeg_[i]);
            if (c)
                d = std::min(d, c->accrualStartDate());
        }
        QL_REQUIRE(d != Date::maxDate(),
                   "not enough information available");
        return d;
    }

    Date Swap::maturity() const {
        Date d = Date::minDate();
        Size i;
        for (i=0; i<firstLeg_.size(); i++)
            d = std::max(d, firstLeg_[i]->date());
        for (i=0; i<secondLeg_.size(); i++)
            d = std::max(d, secondLeg_[i]->date());
        QL_REQUIRE(d != Date::minDate(), "empty swap");
        return d;
    }

    Real Swap::firstLegBPS() const {
        calculate();
        return firstLegBPS_;
    }

    Real Swap::secondLegBPS() const {
        calculate();
        return secondLegBPS_;
    }

    #ifndef QL_DISABLE_DEPRECATED
    TimeBasket Swap::sensitivity(Integer basis) const {
        calculate();
        TimeBasket basket = BasisPointSensitivityBasket(firstLeg_,
                                                        termStructure_,
                                                        basis);
        basket += BasisPointSensitivityBasket(secondLeg_,
                                              termStructure_,
                                              basis);
        return basket;
    }
    #endif

}
