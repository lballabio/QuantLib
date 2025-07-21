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

/*! \file defaultevent.hpp
    \brief Classes for default-event description.
*/

#ifndef quantlib_default_event_hpp
#define quantlib_default_event_hpp

#include <ql/event.hpp>
#include <ql/currency.hpp>
#include <ql/math/comparison.hpp>
#include <ql/experimental/credit/defaulttype.hpp>
#include <ql/experimental/credit/defaultprobabilitykey.hpp>
#include <map>

namespace QuantLib {

    /**
    @class DefaultEvent
    @brief Credit event on a bond of a certain seniority(ies)/currency

      Represents a credit event affecting all bonds with a given \
      seniority and currency. It assumes that all such bonds suffer \
      the event simultaneously.
      Some events affect all seniorities and this has to be encoded
      through a different set of events of the same event type.
      The event is an actual realization, not a contractual reference,
      as such it contains only an atomic type.
    */
    class DefaultEvent : public Event {
      public:
        class DefaultSettlement : public Event {
          public:
            friend class DefaultEvent;
          protected:
            /*! Default settlement events encode the settlement date
                and the recovery rates for the affected
                seniorities. Specific events might require different
                sets of recoveries to be present. The way these
                objects are constructed is a prerogative of the
                particular event class.
            */
            DefaultSettlement(const Date& date,
                              const std::map<Seniority, Real>& recoveryRates);
            /*! When NoSeniority is passed all seniorities are assumed
                to have settled to the recovery passed.
            */
            DefaultSettlement(const Date& date = Date(),
                              Seniority seniority = NoSeniority,
                              Real recoveryRate = 0.4);
          public:
            Date date() const override;
            /*! Returns the recovery rate of a default event which has already
                settled.
            */
            Real recoveryRate(Seniority sen) const;
            void accept(AcyclicVisitor&) override;

          private:
            Date settlementDate_;
            //! Realized recovery rates
            std::map<Seniority, Real> recoveryRates_;
        };
      private:
        // for some reason, gcc chokes on the default parameter below
        // unless we use the typedef
        typedef std::map<Seniority, Real> rate_map;
      public:
        /*! Credit event with optional settlement
            information. Represents a credit event that has taken
            place. Realized events are of an atomic type.  If the
            settlement information is given seniorities present are
            the seniorities/bonds affected by the event.
        */
        DefaultEvent(const Date& creditEventDate,
                     const DefaultType& atomicEvType,
                     Currency curr,
                     Seniority bondsSen,
                     // Settlement information:
                     const Date& settleDate = Date(),
                     const std::map<Seniority, Real>& recoveryRates = rate_map());
        /*! Use NoSeniority to settle to all seniorities with that
            recovery. In that case the event is assumed to have
            affected all seniorities.
        */
        DefaultEvent(const Date& creditEventDate,
                     const DefaultType& atomicEvType,
                     Currency curr,
                     Seniority bondsSen,
                     // Settlement information:
                     const Date& settleDate,
                     Real recoveryRate);

        Date date() const override;
        bool isRestructuring() const { return eventType_.isRestructuring(); }
        bool isDefault() const { return !isRestructuring();}
        bool hasSettled() const {
            return defSettlement_.date() != Date();
        }
        const DefaultSettlement& settlement() const {
            return defSettlement_;
        }
        const DefaultType& defaultType() const {
            return eventType_;
        }
        //! returns the currency of the bond this event refers to.
        const Currency& currency() const {
            return bondsCurrency_;
        }
        //! returns the seniority of the bond that triggered the event.
        Seniority eventSeniority() const {
            return bondsSeniority_;
        }
        /*! returns a value if the event lead to a settlement for the
            requested seniority.  Specializations on the default
            atomics and recoveries could change the default policy.
        */
        virtual Real recoveryRate(Seniority seniority) const {
            if(hasSettled()) {
                return defSettlement_.recoveryRate(seniority);
            }
            return Null<Real>();
        }

        /*! matches the event if this event would trigger a contract
            related to the requested event type.  Notice the
            contractual event types are not neccesarily atomic.
            Notice it does not check seniority or currency only event
            type.  typically used from Issuer
        */
        virtual bool matchesEventType(
                 const ext::shared_ptr<DefaultType>& contractEvType) const {
            // remember we are made of an atomic type.
            // behaviour by default...
            return
                contractEvType->containsRestructuringType(
                    eventType_.restructuringType()) &&
                contractEvType->containsDefaultType(
                    eventType_.defaultType());
        }
        /*! Returns true if this event would trigger a contract with
            the arguments characteristics.
        */
        virtual bool matchesDefaultKey(const DefaultProbKey& contractKey) const;

        void accept(AcyclicVisitor&) override;

      protected:
        Currency bondsCurrency_;
        Date defaultDate_;
        DefaultType eventType_;
        Seniority bondsSeniority_;
        DefaultSettlement defSettlement_;
    };

    /*! Two credit events are the same independently of their
        settlement member data. This has the side effect of
        overwritting different settlements from the same credit event
        when, say, inserting in a map. But on the other hand one given
        event can only have one settlement. This means we can not have
        two restructuring events on a bond on the same date.
    */
    bool operator==(const DefaultEvent& lhs, const DefaultEvent& rhs);

    inline bool operator!=(const DefaultEvent& lhs, const DefaultEvent& rhs) {
        return !(lhs == rhs);
    }

    template<>
    struct earlier_than<DefaultEvent> {
        bool operator()(const DefaultEvent& e1,
                        const DefaultEvent& e2) const {
            return e1.date() < e2.date();
        }
    };


    // ------------------------------------------------------------------------

    class FailureToPayEvent : public DefaultEvent {
      public:
        FailureToPayEvent(const Date& creditEventDate,
                          const Currency& curr,
                          Seniority bondsSen,
                          Real defaultedAmount,
                          // Settlement information:
                          const Date& settleDate,
                          const std::map<Seniority, Real>& recoveryRates);
        FailureToPayEvent(const Date& creditEventDate,
                          const Currency& curr,
                          Seniority bondsSen,
                          Real defaultedAmount,
                          // Settlement information:
                          const Date& settleDate,
                          Real recoveryRates);
        Real amountDefaulted() const {return defaultedAmount_;}
        bool matchesEventType(const ext::shared_ptr<DefaultType>& contractEvType) const override;

      private:
        Real defaultedAmount_;
    };


    // ------------------------------------------------------------------------

    class BankruptcyEvent : public DefaultEvent {
      public:
        BankruptcyEvent(const Date& creditEventDate,
                        const Currency& curr,
                        Seniority bondsSen,
                        // Settlement information:
                        const Date& settleDate,
                        const std::map<Seniority, Real>& recoveryRates);
        BankruptcyEvent(const Date& creditEventDate,
                        const Currency& curr,
                        Seniority bondsSen,
                        // Settlement information:
                        const Date& settleDate,
                        // means same for all
                        Real recoveryRates);
        //! This is a stronger than all event and will trigger all of them.
        bool matchesEventType(const ext::shared_ptr<DefaultType>&) const override { return true; }
    };

}

#endif
