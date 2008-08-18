/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file riskyassetswap.hpp
    \brief Risky asset-swap instrument
*/

#ifndef quantlib_risky_asset_swap_hpp
#define quantlib_risky_asset_swap_hpp

#include <ql/instrument.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Risky asset-swap instrument
    class RiskyAssetSwap : public Instrument {
      public:
        RiskyAssetSwap(bool fixedPayer,
                       Real nominal,
                       const Schedule& fixedSchedule,
                       const Schedule& floatSchedule,
                       const DayCounter& fixedDayCounter,
                       const DayCounter& floatDayCounter,
                       Rate spread,
                       Rate recoveryRate_,
                       const Handle<YieldTermStructure>& yieldTS,
                       const Handle<DefaultProbabilityTermStructure>& defaultTS,
                       Rate coupon = Null<Rate>());

        Real fairSpread ();

        Real floatAnnuity() const;

        Real nominal() { return nominal_; }
        Rate spread() { return spread_; }
        bool fixedPayer() { return fixedPayer_; }

      private:
        void setupExpired() const;
        bool isExpired() const;
        void performCalculations() const;

        Real fixedAnnuity() const;
        Real parCoupon() const;
        Real recoveryValue() const;
        Real riskyBondPrice() const;

        // calculated values
        mutable Real fixedAnnuity_;
        mutable Real floatAnnuity_;
        mutable Real parCoupon_;
        mutable Real recoveryValue_;
        mutable Real riskyBondPrice_;

        // input
        bool fixedPayer_;
        Real nominal_;
        Schedule fixedSchedule_, floatSchedule_;
        DayCounter fixedDayCounter_, floatDayCounter_;
        Rate spread_;
        Rate recoveryRate_;
        Handle<YieldTermStructure> yieldTS_;
        Handle<DefaultProbabilityTermStructure> defaultTS_;
        mutable Real coupon_;
    };

}

#endif

