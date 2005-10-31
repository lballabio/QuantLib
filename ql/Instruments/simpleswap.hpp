/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

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

/*! \file simpleswap.hpp
    \brief Simple fixed-rate vs Libor swap
*/

#ifndef quantlib_simple_swap_hpp
#define quantlib_simple_swap_hpp

#include <ql/Instruments/swap.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>

namespace QuantLib {

    //! Simple fixed-rate vs Libor swap
    /*! \ingroup instruments

        \test
        - the correctness of the returned value is tested by checking
          that the price of a swap paying the fair fixed rate is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap receiving the fair floating-rate
          spread is null.
        - the correctness of the returned value is tested by checking
          that the price of a swap decreases with the paid fixed rate.
        - the correctness of the returned value is tested by checking
          that the price of a swap increases with the received
          floating-rate spread.
        - the correctness of the returned value is tested by checking
          it against a known good value.
    */
    class SimpleSwap : public Swap {
      public:
        class arguments;
        class results;
        SimpleSwap(bool payFixedRate,
                   Real nominal,
                   const Schedule& fixedSchedule,
                   Rate fixedRate,
                   const DayCounter& fixedDayCount,
                   const Schedule& floatSchedule,
                   const boost::shared_ptr<Xibor>& index,
                   Integer indexFixingDays,
                   Spread spread,
                   const Handle<YieldTermStructure>& termStructure);
        // results
        Rate fairRate() const;
        Spread fairSpread() const;
        Real fixedLegBPS() const;
        Real floatingLegBPS() const;
        // inspectors
        Rate fixedRate() const;
        Spread spread() const;
        Real nominal() const;
        bool payFixedRate() const;
        const std::vector<boost::shared_ptr<CashFlow> >& fixedLeg() const;
        const std::vector<boost::shared_ptr<CashFlow> >& floatingLeg() const;
        // other
        void setupArguments(Arguments* args) const;
      private:
        void setupExpired() const;
        void performCalculations() const;
        bool payFixedRate_;
        Rate fixedRate_;
        Spread spread_;
        Real nominal_;
        // results
        mutable Real fixedLegBPS_, floatingLegBPS_;
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };

    //! %Arguments for simple swap calculation
    class SimpleSwap::arguments : public virtual Arguments {
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
    class SimpleSwap::results : public Value {
      public:
        Real fixedLegBPS;
        Real floatingLegBPS;
        Rate fairRate;
        Spread fairSpread;
        results() { reset(); }
        void reset() {
            fixedLegBPS = floatingLegBPS = Null<Real>();
            fairRate = Null<Rate>();
            fairSpread = Null<Spread>();
        }
    };


    // inline definitions

    inline Rate SimpleSwap::fixedRate() const {
        return fixedRate_;
    }

    inline Spread SimpleSwap::spread() const {
        return spread_;
    }

    inline Real SimpleSwap::nominal() const {
        return nominal_;
    }

    inline bool SimpleSwap::payFixedRate() const {
        return payFixedRate_;
    }

    inline const std::vector<boost::shared_ptr<CashFlow> >&
    SimpleSwap::fixedLeg() const {
        return (payFixedRate_ ? firstLeg_ : secondLeg_);
    }

    inline const std::vector<boost::shared_ptr<CashFlow> >&
    SimpleSwap::floatingLeg() const {
        return (payFixedRate_ ? secondLeg_ : firstLeg_);
    }

}


#endif
