/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2016 Peter Caspers

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
#include <ql/optional.hpp>

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
        NonstandardSwap(Swap::Type type,
                        std::vector<Real> fixedNominal,
                        const std::vector<Real>& floatingNominal,
                        Schedule fixedSchedule,
                        std::vector<Real> fixedRate,
                        DayCounter fixedDayCount,
                        Schedule floatingSchedule,
                        ext::shared_ptr<IborIndex> iborIndex,
                        Real gearing,
                        Spread spread,
                        DayCounter floatingDayCount,
                        bool intermediateCapitalExchange = false,
                        bool finalCapitalExchange = false,
                        ext::optional<BusinessDayConvention> paymentConvention = ext::nullopt);
        NonstandardSwap(Swap::Type type,
                        std::vector<Real> fixedNominal,
                        std::vector<Real> floatingNominal,
                        Schedule fixedSchedule,
                        std::vector<Real> fixedRate,
                        DayCounter fixedDayCount,
                        Schedule floatingSchedule,
                        ext::shared_ptr<IborIndex> iborIndex,
                        std::vector<Real> gearing,
                        std::vector<Spread> spread,
                        DayCounter floatingDayCount,
                        bool intermediateCapitalExchange = false,
                        bool finalCapitalExchange = false,
                        ext::optional<BusinessDayConvention> paymentConvention = ext::nullopt);
        //! \name Inspectors
        //@{
        Swap::Type type() const;
        const std::vector<Real> &fixedNominal() const;
        const std::vector<Real> &floatingNominal() const;

        const Schedule &fixedSchedule() const;
        const std::vector<Real> &fixedRate() const;
        const DayCounter &fixedDayCount() const;

        const Schedule &floatingSchedule() const;
        const ext::shared_ptr<IborIndex> &iborIndex() const;
        Spread spread() const;
        Real gearing() const;
        const std::vector<Spread>& spreads() const;
        const std::vector<Real>& gearings() const;
        const DayCounter &floatingDayCount() const;

        BusinessDayConvention paymentConvention() const;

        const Leg &fixedLeg() const;
        const Leg &floatingLeg() const;
        //@}

        //! \name Results
        //@{
        //@}
        // other
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void init();
        void setupExpired() const override;
        Swap::Type type_;
        std::vector<Real> fixedNominal_, floatingNominal_;
        Schedule fixedSchedule_;
        std::vector<Real> fixedRate_;
        DayCounter fixedDayCount_;
        Schedule floatingSchedule_;
        ext::shared_ptr<IborIndex> iborIndex_;
        std::vector<Spread> spread_;
        std::vector<Real> gearing_;
        bool singleSpreadAndGearing_;
        DayCounter floatingDayCount_;
        BusinessDayConvention paymentConvention_;
        const bool intermediateCapitalExchange_;
        const bool finalCapitalExchange_;
        // results
    };

    //! %Arguments for nonstandard swap calculation
    class NonstandardSwap::arguments : public Swap::arguments {
      public:
        arguments() = default;
        Swap::Type type = Swap::Receiver;
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

        ext::shared_ptr<IborIndex> iborIndex;

        std::vector<bool> fixedIsRedemptionFlow;
        std::vector<bool> floatingIsRedemptionFlow;

        void validate() const override;
    };

    //! %Results from nonstandard swap calculation
    class NonstandardSwap::results : public Swap::results {
      public:
        void reset() override;
    };

    class NonstandardSwap::engine
        : public GenericEngine<NonstandardSwap::arguments,
                               NonstandardSwap::results> {};

    // inline definitions

    inline Swap::Type NonstandardSwap::type() const { return type_; }

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

    inline const ext::shared_ptr<IborIndex> &
    NonstandardSwap::iborIndex() const {
        return iborIndex_;
    }

    inline Spread NonstandardSwap::spread() const {
        QL_REQUIRE(singleSpreadAndGearing_,
                   "spread is a vector, use spreads inspector instead");
        return spread_.front();
    }

    inline Real NonstandardSwap::gearing() const {
        QL_REQUIRE(singleSpreadAndGearing_,
                   "gearing is a vector, use gearings inspector instead");
        return gearing_.front();
    }

    inline const std::vector<Spread> &NonstandardSwap::spreads() const {
        return spread_;
    }

    inline const std::vector<Real> &NonstandardSwap::gearings() const {
        return gearing_;
    }

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
