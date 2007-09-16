/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/instruments/swap.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/coupon.hpp>
#include <ql/yieldtermstructure.hpp>

namespace QuantLib {


    Swap::Swap(const Handle<YieldTermStructure>& discountCurve,
               const Leg& firstLeg,
               const Leg& secondLeg)
    : discountCurve_(discountCurve), legs_(2), payer_(2),
      legNPV_(2, 0.0), legBPS_(2, 0.0) {
        legs_[0] = firstLeg;
        legs_[1] = secondLeg;
        payer_[0] = -1.0;
        payer_[1] =  1.0;
        registerWith(discountCurve_);
        Leg::iterator i;
        for (i = legs_[0].begin(); i!= legs_[0].end(); ++i)
            registerWith(*i);
        for (i = legs_[1].begin(); i!= legs_[1].end(); ++i)
            registerWith(*i);
    }

    Swap::Swap(const Handle<YieldTermStructure>& discountCurve,
               const std::vector<Leg>& legs,
               const std::vector<bool>& payer)
    : discountCurve_(discountCurve), legs_(legs), payer_(legs.size(), 1.0),
      legNPV_(legs.size(), 0.0), legBPS_(legs.size(), 0.0) {
        QL_REQUIRE(payer.size()==legs_.size(),
                   "size mismatch between payer (" << payer.size() <<
                   ") and legs (" << legs_.size() << ")");
        registerWith(discountCurve_);
        Leg::iterator i;
        for (Size j=0; j<legs_.size(); ++j) {
            if (payer[j]) payer_[j]=-1.0;
            for (i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                registerWith(*i);
        }
    }

    bool Swap::isExpired() const {
        Date settlement = discountCurve_->referenceDate();
        Leg::const_iterator i;
        for (Size j=0; j<legs_.size(); ++j) {
            for (i = legs_[j].begin(); i!= legs_[j].end(); ++i)
                if (!(*i)->hasOccurred(settlement))
                    return false;
        }
        return true;
    }

    void Swap::setupExpired() const {
        Instrument::setupExpired();
        legBPS_= std::vector<Real>(legs_.size(), 0.0);
        legNPV_= std::vector<Real>(legs_.size(), 0.0);
    }

    void Swap::performCalculations() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "no discounting term structure set to Swap");

        Date d = discountCurve_->referenceDate();

        errorEstimate_ = Null<Real>();
        NPV_ = 0.0;
        for (Size j=0; j<legs_.size(); ++j) {
            //Date settlement = calendar_[j].advance(d, settlementDays_[j], Days);
            Date settlement = d;
            legNPV_[j]= payer_[j]*CashFlows::npv(legs_[j],
                                                 **discountCurve_,
                                                 settlement);
            NPV_ += legNPV_[j] ;
            legBPS_[j] = payer_[j]*CashFlows::bps(legs_[j],
                                                  **discountCurve_,
                                                  settlement);
        }
    }

    Date Swap::startDate() const {
        QL_REQUIRE(!legs_.empty(), "no legs given");
        Date d = CashFlows::startDate(legs_[0]);
        for (Size j=1; j<legs_.size(); ++j)
            d = std::min(d, CashFlows::startDate(legs_[j]));
        return d;
    }

    Date Swap::maturityDate() const {
        QL_REQUIRE(!legs_.empty(), "no legs given");
        Date d = CashFlows::maturityDate(legs_[0]);
        for (Size j=1; j<legs_.size(); ++j)
            d = std::max(d, CashFlows::maturityDate(legs_[j]));
        return d;
    }

}
