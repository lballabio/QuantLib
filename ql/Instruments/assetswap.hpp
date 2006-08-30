/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Chiara Fornarola

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
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>

namespace QuantLib {

    //! Asset swap
    /*! \ingroup instruments

        \test
        - later
    */
    class AssetSwap : public Swap {
      public:
        class arguments;
        class results;
        AssetSwap(bool payFixedRate,
                  Real nominal,
                  const boost::shared_ptr<Bond>& bond,
                  Real bondCleanPrice,
                  const Schedule& floatSchedule,
                  const boost::shared_ptr<Xibor>& index,
                  Spread spread,
                  const DayCounter& floatingDayCount,
                  const Handle<YieldTermStructure>& termStructure);
        // results
        Spread fairSpread() const;
        Real floatingLegBPS() const;
        Real fairPrice() const;
        // inspectors
        Spread spread() const;
        Real nominal() const;
        bool payFixedRate() const;
        const std::vector<boost::shared_ptr<CashFlow> >& fixedLeg() const {
            return legs_[0];
        }
        const std::vector<boost::shared_ptr<CashFlow> >& floatingLeg() const {
            return legs_[1];
        }
        // other
        void setupArguments(Arguments* args) const;
        void fetchResults(const Results*) const;
      private:
        void setupExpired() const;
        void performCalculations() const;
        bool payFixedRate_;
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
    class AssetSwap::arguments : public virtual Arguments {
      public:
        arguments() : payFixed(false),
                      nominal(Null<Real>()),
                      currentFloatingCoupon(Null<Real>()) {}
        bool payFixed;
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
    class AssetSwap::results : public Value {
      public:
        Real floatingLegBPS;
        Spread fairSpread;
        Real fairPrice;
        results() { reset(); }
        void reset() {
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
        return payFixedRate_;
    }

}

#endif
