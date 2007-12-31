/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

/*! \file vanillaswap.hpp
    \brief Simple fixed-rate vs Libor swap
*/

#ifndef quantlib_vanilla_swap_hpp
#define quantlib_vanilla_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    class Schedule;
    class IborIndex;

    //! Plain-vanilla swap
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
    class VanillaSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        class arguments;
        class results;
        class engine;
        VanillaSwap(Type type,
                    Real nominal,
                    const Schedule& fixedSchedule,
                    Rate fixedRate,
                    const DayCounter& fixedDayCount,
                    const Schedule& floatSchedule,
                    const boost::shared_ptr<IborIndex>& iborIndex,
                    Spread spread,
                    const DayCounter& floatingDayCount);
        // results
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Rate fairRate() const;

        Real floatingLegBPS() const;
        Real floatingLegNPV() const;
        Spread fairSpread() const;
        // inspectors
        Rate fixedRate() const;
        const boost::shared_ptr<IborIndex>& iborIndex() const;
        Spread spread() const;
        Real nominal() const;
        Type type() const;
        const Leg& fixedLeg() const {
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
        Type type_;
        Rate fixedRate_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Spread spread_;
        Real nominal_;
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };


    //! %Arguments for simple swap calculation
    class VanillaSwap::arguments : public Swap::arguments {
      public:
        arguments() : type(Receiver),
                      nominal(Null<Real>()) {}
        Type type;
        Real nominal;

        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;

        std::vector<Real> fixedCoupons;
        std::vector<Spread> floatingSpreads;
        std::vector<Real> floatingCoupons;
        void validate() const;
    };

    //! %Results from simple swap calculation
    class VanillaSwap::results : public Swap::results {
      public:
        Rate fairRate;
        Spread fairSpread;
        void reset();
    };

    class VanillaSwap::engine : public GenericEngine<VanillaSwap::arguments,
                                                     VanillaSwap::results> {};


    // inline definitions

    inline Rate VanillaSwap::fixedRate() const {
        return fixedRate_;
    }

    inline const boost::shared_ptr<IborIndex>& VanillaSwap::iborIndex() const {
        return iborIndex_;
    }


    inline Spread VanillaSwap::spread() const {
        return spread_;
    }

    inline Real VanillaSwap::nominal() const {
        return nominal_;
    }

    inline VanillaSwap::Type VanillaSwap::type() const {
        return type_;
    }

    inline std::ostream& operator<<(std::ostream& out,
                                    VanillaSwap::Type type) {
        switch (type) {
          case VanillaSwap::Payer:
            return out << "Payer";
          case VanillaSwap::Receiver:
            return out << "Receiver";
          default:
            QL_FAIL("unknown option type");
        }
    }

}

#endif
