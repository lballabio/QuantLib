/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2010 Andre Miemiec

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

/*! \file irregularswap.hpp
    \brief Irregular fixed-rate vs Libor swap
*/

#ifndef quantlib_irregular_swap_hpp
#define quantlib_irregular_swap_hpp

#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class IborIndex;

    //! Irregular swap: fixed vs floating leg
    class IrregularSwap : public Swap {
      public:
        class arguments;
        class results;
        class engine;
        IrregularSwap(
            Type type,
            const Leg& fixLeg,
            const Leg& floatLeg);
        //! \name Inspectors
        //@{
        Type type() const;

        const Leg& fixedLeg() const;
        const Leg& floatingLeg() const;
        //@}

        //! \name Results
        //@{
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Rate fairRate() const;

        Real floatingLegBPS() const;
        Real floatingLegNPV() const;
        Spread fairSpread() const;
        //@}
        // other
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;
        Type type_;

        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };


    //! %Arguments for irregular-swap calculation
    class IrregularSwap::arguments : public Swap::arguments {
      public:
        arguments() = default;
        Type type = Receiver;


        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Real> fixedCoupons;
        std::vector<Real> fixedNominals;

        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Real> floatingNominals;
        std::vector<Spread> floatingSpreads;
        std::vector<Real> floatingCoupons;

        void validate() const override;
    };

    //! %Results from irregular-swap calculation
    class IrregularSwap::results : public Swap::results {
      public:
        Rate fairRate;
        Spread fairSpread;
        void reset() override;
    };

    class IrregularSwap::engine : public GenericEngine<IrregularSwap::arguments,
                                                       IrregularSwap::results> {};


    // inline definitions

    inline Swap::Type IrregularSwap::type() const {
        return type_;
    }

    inline const Leg& IrregularSwap::fixedLeg() const {
        return legs_[0];
    }

    inline const Leg& IrregularSwap::floatingLeg() const {
        return legs_[1];
    }

}

#endif
