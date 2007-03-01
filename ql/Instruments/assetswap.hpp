/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Chiara Fornarola
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file assetswap.hpp
    \brief Bullet Bond vs Libor swap
*/

#ifndef quantlib_asset_swap_hpp
#define quantlib_asset_swap_hpp

#include <ql/Instruments/swap.hpp>
#include <ql/Instruments/bond.hpp>
#include <ql/Indexes/iborindex.hpp>
#include <ql/interestrate.hpp>
#include <ql/schedule.hpp>

namespace QuantLib {

    /*! for mechanics of Par Asset Swap and Market Asset Swap refer to
        "Introduction to Asset Swap", Lehman Brothers European Fixed
        Income Research - January 2000, D. O'Kane

        \ingroup instruments
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
                  bool parSwap = true);
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
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
        mutable Real fairPrice_;
    };


    //! %Arguments for asset swap calculation
    class AssetSwap::arguments : public virtual PricingEngine::arguments {
      public:
        arguments() : nominal(Null<Real>()),
                      currentFloatingCoupon(Null<Real>()) {}
        Real nominal;
        std::vector<Time> fixedResetTimes;
        std::vector<Time> fixedPayTimes;
        std::vector<Real> fixedCoupons;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Time> floatingResetTimes;
        std::vector<Time> floatingFixingTimes;
        std::vector<Time> floatingPayTimes;
        std::vector<Spread> floatingSpreads;
        Real currentFloatingCoupon;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class AssetSwap::results : public Instrument::results {
      public:
        Real floatingLegBPS;
        Spread fairSpread;
        Real fairPrice;
        void reset() {
            Instrument::results::reset();
            floatingLegBPS = Null<Real>();
            fairSpread = Null<Spread>();
            fairPrice = Null<Real>();
        }
    };


    // inline definitions

    inline Spread AssetSwap::spread() const {
        return spread_;
    }

    inline Real AssetSwap::nominal() const {
        return nominal_;
    }

    inline bool AssetSwap::payFixedRate() const {
        return (payer_[0]==-1.0);
    }

}

#endif
