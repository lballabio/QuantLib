/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2011 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007, 2008 StatPro Italia srl

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
#include <ql/cashflows/floatingratecoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ostream>

namespace QuantLib {

    Swap::Swap(const Leg& firstLeg,
               const Leg& secondLeg)
    : legs_(2), payer_(2),
      legNPV_(2, 0.0), legBPS_(2, 0.0),
      startDiscounts_(2, 0.0), endDiscounts_(2, 0.0),
      npvDateDiscount_(0.0) {
        legs_[0] = firstLeg;
        legs_[1] = secondLeg;
        payer_[0] = -1.0;
        payer_[1] =  1.0;
        for (auto& i : legs_[0])
            registerWith(i);
        for (auto& i : legs_[1])
            registerWith(i);
    }

    Swap::Swap(const std::vector<Leg>& legs,
               const std::vector<bool>& payer)
    : legs_(legs), payer_(legs.size(), 1.0),
      legNPV_(legs.size(), 0.0), legBPS_(legs.size(), 0.0),
      startDiscounts_(legs.size(), 0.0), endDiscounts_(legs.size(), 0.0),
      npvDateDiscount_(0.0) {
        QL_REQUIRE(payer.size()==legs_.size(),
                   "size mismatch between payer (" << payer.size() <<
                   ") and legs (" << legs_.size() << ")");
        for (Size j=0; j<legs_.size(); ++j) {
            if (payer[j]) payer_[j]=-1.0;
            for (auto& i : legs_[j])
                registerWith(i);
        }
    }

    Swap::Swap(Size legs)
    : legs_(legs), payer_(legs),
      legNPV_(legs, 0.0), legBPS_(legs, 0.0),
      startDiscounts_(legs, 0.0), endDiscounts_(legs, 0.0),
      npvDateDiscount_(0.0) {}


    bool Swap::isExpired() const {
        for (const auto& leg : legs_) {
            Leg::const_iterator i;
            for (i = leg.begin(); i != leg.end(); ++i)
                if (!(*i)->hasOccurred())
                    return false;
        }
        return true;
    }

    void Swap::setupExpired() const {
        Instrument::setupExpired();
        std::fill(legBPS_.begin(), legBPS_.end(), 0.0);
        std::fill(legNPV_.begin(), legNPV_.end(), 0.0);
        std::fill(startDiscounts_.begin(), startDiscounts_.end(), 0.0);
        std::fill(endDiscounts_.begin(), endDiscounts_.end(), 0.0);
        npvDateDiscount_ = 0.0;
    }

    void Swap::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<Swap::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->legs = legs_;
        arguments->payer = payer_;
    }

    void Swap::fetchResults(const PricingEngine::results* r) const {
        Instrument::fetchResults(r);

        const auto* results = dynamic_cast<const Swap::results*>(r);
        QL_REQUIRE(results != nullptr, "wrong result type");

        if (!results->legNPV.empty()) {
            QL_REQUIRE(results->legNPV.size() == legNPV_.size(),
                       "wrong number of leg NPV returned");
            legNPV_ = results->legNPV;
        } else {
            std::fill(legNPV_.begin(), legNPV_.end(), Null<Real>());
        }

        if (!results->legBPS.empty()) {
            QL_REQUIRE(results->legBPS.size() == legBPS_.size(),
                       "wrong number of leg BPS returned");
            legBPS_ = results->legBPS;
        } else {
            std::fill(legBPS_.begin(), legBPS_.end(), Null<Real>());
        }

        if (!results->startDiscounts.empty()) {
            QL_REQUIRE(results->startDiscounts.size() == startDiscounts_.size(),
                       "wrong number of leg start discounts returned");
            startDiscounts_ = results->startDiscounts;
        } else {
            std::fill(startDiscounts_.begin(), startDiscounts_.end(),
                                                    Null<DiscountFactor>());
        }

        if (!results->endDiscounts.empty()) {
            QL_REQUIRE(results->endDiscounts.size() == endDiscounts_.size(),
                       "wrong number of leg end discounts returned");
            endDiscounts_ = results->endDiscounts;
        } else {
            std::fill(endDiscounts_.begin(), endDiscounts_.end(),
                                                    Null<DiscountFactor>());
        }

        if (results->npvDateDiscount != Null<DiscountFactor>()) {
            npvDateDiscount_ = results->npvDateDiscount;
        } else {
            npvDateDiscount_ = Null<DiscountFactor>();
        }
    }

    Size Swap::numberOfLegs() const { return legs_.size(); }

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

    void Swap::deepUpdate() {
        for (auto& leg : legs_) {
            for (auto& k : leg) {
                std::shared_ptr<LazyObject> f = std::dynamic_pointer_cast<LazyObject>(k);
                if (f != nullptr)
                    f->update();
            }
        }
        update();
    }

    void Swap::arguments::validate() const {
        QL_REQUIRE(legs.size() == payer.size(),
                   "number of legs and multipliers differ");
    }

    void Swap::results::reset() {
        Instrument::results::reset();
        legNPV.clear();
        legBPS.clear();
        startDiscounts.clear();
        endDiscounts.clear();
        npvDateDiscount = Null<DiscountFactor>();
    }

    std::ostream& operator<<(std::ostream& out, Swap::Type t) {
        switch (t) {
          case Swap::Payer:
            return out << "Payer";
          case Swap::Receiver:
            return out << "Receiver";
          default:
            QL_FAIL("unknown Swap::Type(" << Integer(t) << ")");
        }
    }

}
