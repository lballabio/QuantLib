/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file assetswap.hpp
    \brief Bullet bond vs Libor swap
*/

#ifndef quantlib_asset_swap_hpp
#define quantlib_asset_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/instruments/bond.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    class IborIndex;

    //! Bullet bond vs %Libor swap
    /*! for mechanics of par asset swap and market asset swap, refer to
        "Introduction to Asset Swap", Lehman Brothers European Fixed
        Income Research - January 2000, D. O'Kane

        \ingroup instruments

        \bug fair prices are not calculated correctly when using
             indexed coupons.
    */
    class AssetSwap : public Swap {
      public:
        class arguments;
        class results;
        AssetSwap(bool payFixedRate,
                  const boost::shared_ptr<Bond>& bond,
                  Real bondCleanPrice,
                  const boost::shared_ptr<IborIndex>& index,
                  Spread spread,
                  const Handle<YieldTermStructure>& discountCurve,
                  const Schedule& floatSchedule = Schedule(),
                  const DayCounter& floatingDayCount = DayCounter(),
                  bool parAssetSwap = true);
        // results
        Spread fairSpread() const;
        Real floatingLegBPS() const;
        Real fairPrice() const;
        // inspectors
        Spread spread() const;
        Real nominal() const;
        bool payFixedRate() const;
        const Leg& bondLeg() const {
            return legs_[0];
        }
        const Leg& floatingLeg() const {
            return legs_[1];
        }
        // other
        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results*) const;
      private:
        void setupExpired() const;
        void performCalculations() const;
        Spread spread_;
        Real nominal_;
        Date upfrontDate_;
        Real bondCleanPrice_;
        Handle<YieldTermStructure> discountCurve_;
        // results
        mutable Spread fairSpread_;
        mutable Real fairPrice_;
    };


    //! %Arguments for asset swap calculation
    class AssetSwap::arguments : public Swap::arguments {
      public:
        arguments() : nominal(Null<Real>()),
                      currentFloatingCoupon(Null<Rate>()) {}
        Real nominal;
        Date settlementDate;
        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Real> fixedCoupons;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;
        std::vector<Spread> floatingSpreads;
        Rate currentFloatingCoupon;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class AssetSwap::results : public Swap::results {
      public:
        Spread fairSpread;
        Real fairPrice;
        void reset();
    };


    // inline definitions

    inline Spread AssetSwap::spread() const {
        return spread_;
    }

    inline Real AssetSwap::nominal() const {
        return nominal_;
    }

    inline bool AssetSwap::payFixedRate() const {
        return (payer_[0] == -1.0);
    }

}

#endif
