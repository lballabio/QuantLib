/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl
 Copyright (C) 2009 Jose Aparicio

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

#include <ql/experimental/credit/defaultevent.hpp>
#include <ql/experimental/credit/recoveryratequote.hpp>
#include <ql/patterns/visitor.hpp>
#include <ql/settings.hpp>
#include <utility>

namespace QuantLib {

    Date DefaultEvent::date() const {
        return defaultDate_;
    }

    void DefaultEvent::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DefaultEvent>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Event::accept(v);
    }

    // They will be sorted by settlement date
    Date DefaultEvent::DefaultSettlement::date() const {
        return settlementDate_;
    }

    void DefaultEvent::DefaultSettlement::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<DefaultEvent::DefaultSettlement>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            Event::accept(v);
    }

    DefaultEvent::DefaultSettlement::DefaultSettlement(
        const Date& date,
        const std::map<Seniority, Real>& recoveryRates )
    : settlementDate_(date), recoveryRates_(recoveryRates) {
        QL_REQUIRE(recoveryRates.find(NoSeniority) ==
            recoveryRates.end(),
            "NoSeniority is not a valid realized seniority.");
    }

    DefaultEvent::DefaultSettlement::DefaultSettlement(
        const Date& date,
        Seniority seniority,
        const Real recoveryRate)
    : settlementDate_(date), recoveryRates_(makeIsdaConvMap()) {
        if (seniority == NoSeniority) {
            for (auto& i : recoveryRates_) {
                i.second = recoveryRate;
            }
        } else {
            recoveryRates_[seniority] = recoveryRate;
        }
    }

    Real DefaultEvent::DefaultSettlement::recoveryRate(
        Seniority sen) const {
        // expensive require cause called often...... fix me
        QL_REQUIRE(sen != NoSeniority,
            "NoSeniority is not valid for recovery rate request.");
        auto itmatch = recoveryRates_.find(sen);
        if(itmatch != recoveryRates_.end()) {
            return itmatch->second;
        }else{
            return Null<Real>();
        }
    }

    DefaultEvent::DefaultEvent(const Date& creditEventDate,
                               const DefaultType& atomicEvType,
                               Currency curr,
                               Seniority bondsSen,
                               // Settlement information:
                               const Date& settleDate,
                               const std::map<Seniority, Real>& recoveryRates)
    : bondsCurrency_(std::move(curr)), defaultDate_(creditEventDate), eventType_(atomicEvType),
      bondsSeniority_(bondsSen),
      defSettlement_(settleDate, recoveryRates.empty() ? makeIsdaConvMap() : recoveryRates) {
        if (settleDate != Date()) {// has settled
            QL_REQUIRE(settleDate >= creditEventDate,
              "Settlement date should be after default date.");
            QL_REQUIRE(recoveryRates.find(bondsSen) != recoveryRates.end(),
              "Settled events must contain the seniority of the default");
        }
    }

    DefaultEvent::DefaultEvent(const Date& creditEventDate,
                               const DefaultType& atomicEvType,
                               Currency curr,
                               Seniority bondsSen,
                               // Settlement information:
                               const Date& settleDate,
                               Real recoveryRate)
    : bondsCurrency_(std::move(curr)), defaultDate_(creditEventDate), eventType_(atomicEvType),
      bondsSeniority_(bondsSen), defSettlement_(settleDate, bondsSen, recoveryRate) {
        if (settleDate != Date()) {
            QL_REQUIRE(settleDate >= creditEventDate,
            "Settlement date should be after default date.");
        }
    }

    bool DefaultEvent::matchesDefaultKey(
        const DefaultProbKey& contractKey) const {
        if(bondsCurrency_ != contractKey.currency()) return false;
        // a contract with NoSeniority matches all events
        if((bondsSeniority_ != contractKey.seniority())
            && (contractKey.seniority() != NoSeniority))
            return false;
        // loop on all event types in the contract and chek if we match any,
        //   calls derived types
        for(Size i=0; i<contractKey.size(); i++) {
            if(this->matchesEventType(contractKey.eventTypes()[i])) return true;
        }
        return false;
    }



    bool operator==(const DefaultEvent& lhs, const DefaultEvent& rhs) {
        return (lhs.currency() == rhs.currency()) &&
            (lhs.defaultType() == rhs.defaultType()) &&
            (lhs.date() == rhs.date()) &&
            (lhs.eventSeniority() == rhs.eventSeniority());
    }


    bool FailureToPayEvent::matchesEventType(
        const ext::shared_ptr<DefaultType>& contractEvType) const {
        ext::shared_ptr<FailureToPay> eveType =
            ext::dynamic_pointer_cast<FailureToPay>(contractEvType);
        // this chekcs the atomic types, no need to call parents method
        if(!eveType) return false;
        if(defaultedAmount_ < eveType->amountRequired()) return false;
        Date today = Settings::instance().evaluationDate();
        return this->hasOccurred(today - eveType->gracePeriod(), true);
    }



    FailureToPayEvent::FailureToPayEvent(const Date& creditEventDate,
                                         const Currency& curr,
                                         Seniority bondsSen,
                                         Real defaultedAmount,
                                         // Settlement information:
                                         const Date& settleDate,
                                         const std::map<Seniority, Real>&
                                            recoveryRates)
    : DefaultEvent(creditEventDate,
                   DefaultType(AtomicDefault::FailureToPay,
                               Restructuring::XR),
                   curr,
                   bondsSen,
                   settleDate,
                   recoveryRates),
      defaultedAmount_(defaultedAmount) { }

    FailureToPayEvent::FailureToPayEvent(const Date& creditEventDate,
                                         const Currency& curr,
                                         Seniority bondsSen,
                                         Real defaultedAmount,
                                         // Settlement information:
                                         const Date& settleDate,
                                         Real recoveryRates)
    : DefaultEvent(creditEventDate,
                   DefaultType(AtomicDefault::FailureToPay,
                               Restructuring::XR),
                   curr,
                   bondsSen,
                   settleDate,
                   recoveryRates),
      defaultedAmount_(defaultedAmount) { }



    BankruptcyEvent::BankruptcyEvent(const Date& creditEventDate,
                                     const Currency& curr,
                                     Seniority bondsSen,
                                     // Settlement information:
                                     const Date& settleDate,
                                     const std::map<Seniority, Real>&
                                        recoveryRates)
    : DefaultEvent(creditEventDate,
                   DefaultType(AtomicDefault::Bankruptcy,
                               Restructuring::XR),
                   curr,
                   bondsSen,
                   settleDate,
                   recoveryRates) {
        if(hasSettled()) {
            QL_REQUIRE(recoveryRates.size() == makeIsdaConvMap().size(),
              "Bankruptcy event should have settled for all seniorities.");
        }
    }

    BankruptcyEvent::BankruptcyEvent(const Date& creditEventDate,
                                     const Currency& curr,
                                     Seniority bondsSen,
                                     // Settlement information:
                                     const Date& settleDate,
                                     // means same for all
                                     Real recoveryRates)
    : DefaultEvent(creditEventDate,
                   DefaultType(AtomicDefault::Bankruptcy,
                               Restructuring::XR),
                   curr,
                   bondsSen,
                   settleDate,
                   recoveryRates) { }

}
