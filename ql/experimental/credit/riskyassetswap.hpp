/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008, 2009 Roland Lichters

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
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>
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

        Real nominal() const { return nominal_; }
        Rate spread() const { return spread_; }
        bool fixedPayer() const { return fixedPayer_; }

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


    // risky-asset-swap helper for probability-curve bootstrap
    class AssetSwapHelper : public DefaultProbabilityHelper {
      public:
        AssetSwapHelper(const Handle<Quote>& spread,
                        const Period& tenor,
                        Natural settlementDays,
                        const Calendar& calendar,
                        const Period& fixedPeriod,
                        BusinessDayConvention fixedConvention,
                        const DayCounter& fixedDayCount,
                        const Period& floatPeriod,
                        BusinessDayConvention floatConvention,
                        const DayCounter& floatDayCount,
                        Real recoveryRate,
                        const RelinkableHandle<YieldTermStructure>& yieldTS,
                        const Period& integrationStepSize = Period());
        Real impliedQuote() const;
        void setTermStructure(DefaultProbabilityTermStructure*);

      private:
        void update();
        void initializeDates();

        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        BusinessDayConvention fixedConvention_;
        Period fixedPeriod_;
        DayCounter fixedDayCount_;
        BusinessDayConvention floatConvention_;
        Period floatPeriod_;
        DayCounter floatDayCount_;
        Real recoveryRate_;
        RelinkableHandle<YieldTermStructure> yieldTS_;
        Period integrationStepSize_;

        Date evaluationDate_;
        ext::shared_ptr<RiskyAssetSwap> asw_;
        RelinkableHandle<DefaultProbabilityTermStructure> probability_;
    };

}

#endif

