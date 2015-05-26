/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file nonstandardswap.hpp
    \brief vanilla swap but possibly with period dependent nominal and strike
*/

#ifndef quantlib_nonstandard_swap_hpp
#define quantlib_nonstandard_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <boost/optional.hpp>

namespace QuantLib {

    class IborIndex;
    class SwapIndex;

    //! nonstandard swap

    class NonstandardSwap : public Swap {
      public:
        class arguments;
        class results;
        class engine;
        NonstandardSwap(const VanillaSwap &fromVanilla);
        NonstandardSwap(
            const VanillaSwap::Type type, const std::vector<Real> &fixedNominal,
            const std::vector<Real> &floatingNominal,
            const Schedule &fixedSchedule, const std::vector<Real> &fixedRate,
            const DayCounter &fixedDayCount, const Schedule &floatingSchedule,
            const boost::shared_ptr<IborIndex> &iborIndex, const Real gearing,
            const Spread spread, const DayCounter &floatingDayCount,
            const bool intermediateCapitalExchange = false,
            const bool finalCapitalExchange = false,
            boost::optional<BusinessDayConvention> paymentConvention =
                boost::none);
        //! \name Inspectors
        //@{
        VanillaSwap::Type type() const;
        const std::vector<Real> &fixedNominal() const;
        const std::vector<Real> &floatingNominal() const;

        const Schedule &fixedSchedule() const;
        const std::vector<Real> &fixedRate() const;
        const DayCounter &fixedDayCount() const;

        const Schedule &floatingSchedule() const;
        const boost::shared_ptr<IborIndex> &iborIndex() const;
        const Spread spread() const;
        const Real gearing() const;
        const DayCounter &floatingDayCount() const;

        BusinessDayConvention paymentConvention() const;

        const Leg &fixedLeg() const;
        const Leg &floatingLeg() const;
        //@}

        //! \name Results
        //@{
        //@}
        // other
        void setupArguments(PricingEngine::arguments *args) const;
        void fetchResults(const PricingEngine::results *) const;

      private:
        void init();
        void setupExpired() const;
        VanillaSwap::Type type_;
        std::vector<Real> fixedNominal_, floatingNominal_;
        Schedule fixedSchedule_;
        std::vector<Real> fixedRate_;
        DayCounter fixedDayCount_;
        Schedule floatingSchedule_;
        boost::shared_ptr<IborIndex> iborIndex_;
        Spread spread_;
        Real gearing_;
        DayCounter floatingDayCount_;
        BusinessDayConvention paymentConvention_;
        const bool intermediateCapitalExchange_;
        const bool finalCapitalExchange_;
        // results
    };

    //! %Arguments for nonstandard swap calculation
    class NonstandardSwap::arguments : public Swap::arguments {
      public:
        arguments() : type(VanillaSwap::Receiver) {}
        VanillaSwap::Type type;
        std::vector<Real> fixedNominal, floatingNominal;

        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;

        std::vector<Real> fixedCoupons;
        std::vector<Real> fixedRate;
        std::vector<Spread> floatingSpreads;
        std::vector<Real> floatingGearings;
        std::vector<Real> floatingCoupons;

        boost::shared_ptr<IborIndex> iborIndex;

        std::vector<bool> fixedIsRedemptionFlow;
        std::vector<bool> floatingIsRedemptionFlow;

        void validate() const;
    };

    //! %Results from nonstandard swap calculation
    class NonstandardSwap::results : public Swap::results {
      public:
        void reset();
    };

    class NonstandardSwap::engine
        : public GenericEngine<NonstandardSwap::arguments,
                               NonstandardSwap::results> {};

    // inline definitions

    inline VanillaSwap::Type NonstandardSwap::type() const { return type_; }

    inline const std::vector<Real> &NonstandardSwap::fixedNominal() const {
        return fixedNominal_;
    }

    inline const std::vector<Real> &NonstandardSwap::floatingNominal() const {
        return floatingNominal_;
    }

    inline const Schedule &NonstandardSwap::fixedSchedule() const {
        return fixedSchedule_;
    }

    inline const std::vector<Real> &NonstandardSwap::fixedRate() const {
        return fixedRate_;
    }

    inline const DayCounter &NonstandardSwap::fixedDayCount() const {
        return fixedDayCount_;
    }

    inline const Schedule &NonstandardSwap::floatingSchedule() const {
        return floatingSchedule_;
    }

    inline const boost::shared_ptr<IborIndex> &
    NonstandardSwap::iborIndex() const {
        return iborIndex_;
    }

    inline const Spread NonstandardSwap::spread() const { return spread_; }

    inline const Real NonstandardSwap::gearing() const { return gearing_; }

    inline const DayCounter &NonstandardSwap::floatingDayCount() const {
        return floatingDayCount_;
    }

    inline BusinessDayConvention NonstandardSwap::paymentConvention() const {
        return paymentConvention_;
    }

    inline const Leg &NonstandardSwap::fixedLeg() const { return legs_[0]; }

    inline const Leg &NonstandardSwap::floatingLeg() const { return legs_[1]; }
}

#endif
