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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file defaulttype.hpp
    \brief Classes for default-event description.
*/


#ifndef quantlib_default_type_hpp
#define quantlib_default_type_hpp

#include <ql/time/period.hpp>

namespace QuantLib {

    //! Seniority of a bond.
    /*! They are also ISDA tier/seniorities used for CDS conventional
        spreads.
    */
    enum Seniority {
        SecDom = 0,
        SnrFor,
        SubLT2,
        JrSubT2,
        PrefT1,
        // Unassigned value, allows for default RR quote
        NoSeniority,
        // markit parlance
        SeniorSec     = SecDom,
        SeniorUnSec   = SnrFor,
        SubTier1      = PrefT1,
        SubUpperTier2 = JrSubT2,
        SubLoweTier2  = SubLT2
    };


    //! Atomic (single contractual event) default events.
    /*! Default types defined as enum to allow easy aggregation of
        types. Theres an event algebra logic by default provided by
        DefaultType. If your new type requires more sofisticated test
        you need to derive from it as in FailureToPay
    */
    struct AtomicDefault {
        enum Type {
            // Includes one of the restructuring cases
            Restructuring = 0,
            Bankruptcy,
            FailureToPay,
            RepudiationMoratorium,
            Acceleration,
            Default,
            // synonyms
            ObligationAcceleration = Acceleration,
            ObligationDefault = Default,
            CrossDefault = Default,
            // Other non-isda
            Downgrade,   // Non-ISDA, not in FpML
            MergerEvent  // Non-ISDA, not in FpML
        };
    };


    // these could be merged with the ones above if not because
    //   restructuring types can not be combined together.

    //! Restructuring type
    struct Restructuring {
        enum Type {
            NoRestructuring = 0,
            ModifiedRestructuring,
            ModifiedModifiedRestructuring,
            FullRestructuring,
            AnyRestructuring,
            // Markit notation:
            XR = NoRestructuring,
            MR = ModifiedRestructuring,
            MM = ModifiedModifiedRestructuring,
            CR = FullRestructuring
        };
    };


    //! Atomic credit-event type.
    /*! This class encapsulates the ISDA default contractual types and
        their combinations. Non-atomicity works only at the atomic
        type level, obviating the specific event characteristics which
        it is accounted for only in derived classes.
    */
    class DefaultType {
      public:
        explicit DefaultType(AtomicDefault::Type defType =
                                                    AtomicDefault::Bankruptcy,
                             Restructuring::Type restType = Restructuring::XR);

        virtual ~DefaultType() = default;

        AtomicDefault::Type defaultType() const {
            return defTypes_;
        }
        Restructuring::Type restructuringType() const {return restrType_;}
        bool isRestructuring() const {
            return restrType_ != Restructuring::NoRestructuring;
        }

        // bool isAtomic() const { return defTypes_.size() == 1;}

        /*! Returns true if one or a set of event types is within this
            one and as such will be recognised as a trigger. Not the
            same as equality.

            Notice that these methods do not include any event logical
            hierarchy. The match is in a strict sense. If event B is
            contained in (implied by) event A this would not send a
            match. This policies should be implemented at the
            CreditEvent class, which is polymorphic.
        */
        bool containsDefaultType(AtomicDefault::Type defType) const {
            return defTypes_ ==  defType;
        }

        bool containsRestructuringType(Restructuring::Type resType) const {
            return (restrType_ == resType) ||
                (Restructuring::AnyRestructuring == resType);
        }
    protected:
        //std::set<AtomicDefault::Type> defTypes_;
        AtomicDefault::Type defTypes_;
        Restructuring::Type restrType_;
    };


    /*! Equality is the criteria for indexing the curves. This depends
        only on the atomic types and not on idiosincracies of derived
        type as mentioned in the functional documentation (specific
        event characteristics are relevant to credit event matching
        but not to the probability meaning).  operator== is also used
        to remove duplicates in some containers. This ensures we do
        not have two equal events (despite having different
        characteristics) in those containers. This makes sense, theres
        no logic in having two FailureToPay in a contract even if they
        have different characteristics.
    */
    bool operator==(const DefaultType& lhs, const DefaultType& rhs);



    //! Failure to Pay atomic event type.
    class FailureToPay : public DefaultType {
      public:
        // Only atomic construction.
        // Amount contract by default is in dollars as per ISDA doc and not
        //   the contract curr. Theres an issue here...... FIX ME
        explicit FailureToPay(const Period& grace,
                              Real amount = 1.e+6)
        : DefaultType(AtomicDefault::FailureToPay, Restructuring::XR),
          gracePeriod_(grace), amountRequired_(amount) {}

        Real amountRequired() const {return amountRequired_;}
        const Period& gracePeriod() const {return gracePeriod_;}
      private:
        // Grace period to consider the event. If payment occurs during
        // the period the event should be removed from its container.
        Period gracePeriod_;
        // Minimum default amount triggering the event
        Real amountRequired_;
    };

}

#endif
