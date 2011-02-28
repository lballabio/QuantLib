/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Chiara Fornarola
 Copyright (C) 2007, 2009 Ferdinando Ametrano
 Copyright (C) 2007, 2009 StatPro Italia srl

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
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    class IborIndex;
    class Schedule;

    //! Bullet bond vs %Libor swap
    /*! for mechanics of par asset swap and market asset swap, refer to
        "Introduction to Asset Swap", Lehman Brothers European Fixed
        Income Research - January 2000, D. O'Kane

        \ingroup instruments

        \warning bondCleanPrice must be the (forward) price at the
                 floatSchedule start date

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
                  const Schedule& floatSchedule = Schedule(),
                  const DayCounter& floatingDayCount = DayCounter(),
                  bool parAssetSwap = true);
        // results
        Spread fairSpread() const;
        Real floatingLegBPS() const;
        Real fairCleanPrice() const;
        // inspectors
        bool parSwap() const { return parSwap_; }
        Spread spread() const { return spread_; }
        const boost::shared_ptr<Bond>& bond() const { return bond_; }
        bool payFixedRate() const { return (payer_[0] == -1.0); }
        const Leg& bondLeg() const { return legs_[0]; }
        const Leg& floatingLeg() const { return legs_[1]; }
        // other
        void setupArguments(PricingEngine::arguments* args) const;
        void fetchResults(const PricingEngine::results*) const;
      private:
        void setupExpired() const;
        boost::shared_ptr<Bond> bond_;
        Real bondCleanPrice_;
        Spread spread_;
        bool parSwap_;
        Date upfrontDate_;
        // results
        mutable Spread fairSpread_;
        mutable Real fairCleanPrice_;
    };


    //! %Arguments for asset swap calculation
    class AssetSwap::arguments : public Swap::arguments {
      public:
        arguments() {}
        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Real> fixedCoupons;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;
        std::vector<Spread> floatingSpreads;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class AssetSwap::results : public Swap::results {
      public:
        Spread fairSpread;
        Real fairCleanPrice;
        void reset();
    };

}

#endif
