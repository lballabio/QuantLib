/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2006, 2008 Ferdinando Ametrano

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

/*! \file fixedvsfloatingswap.hpp
    \brief Fixed-rate vs floating-rate swap
*/

#ifndef quantlib_fixed_vs_floating_swap_hpp
#define quantlib_fixed_vs_floating_swap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    class IborIndex;

    //! Fixed vs floating swap
    /*! \ingroup instruments

        If no payment convention is passed, the convention of the
        floating-rate schedule is used.

        \warning if <tt>Settings::includeReferenceDateCashFlows()</tt>
                 is set to <tt>true</tt>, payments occurring at the
                 settlement date of the swap might be included in the
                 NPV and therefore affect the fair-rate and
                 fair-spread calculation. This might not be what you
                 want.
    */
    class FixedVsFloatingSwap : public Swap {
      public:
        class arguments;
        class results;
        class engine;
        FixedVsFloatingSwap(Type type,
                            std::vector<Real> fixedNominals,
                            Schedule fixedSchedule,
                            Rate fixedRate,
                            DayCounter fixedDayCount,
                            std::vector<Real> floatingNominals,
                            Schedule floatingSchedule,
                            ext::shared_ptr<IborIndex> iborIndex,
                            Spread spread,
                            DayCounter floatingDayCount,
                            ext::optional<BusinessDayConvention> paymentConvention = ext::nullopt,
                            Natural paymentLag = 0,
                            const Calendar& paymentCalendar = Calendar());
        //! \name Inspectors
        //@{
        Type type() const;

        /*! This throws if the nominal is not constant across coupons. */
        Real nominal() const;
        /*! This throws if the nominals are not the same for the two legs. */
        const std::vector<Real>& nominals() const;

        const std::vector<Real>& fixedNominals() const;
        const Schedule& fixedSchedule() const;
        Rate fixedRate() const;
        const DayCounter& fixedDayCount() const;

        const std::vector<Real>& floatingNominals() const;
        const Schedule& floatingSchedule() const;
        const ext::shared_ptr<IborIndex>& iborIndex() const;
        Spread spread() const;
        const DayCounter& floatingDayCount() const;

        BusinessDayConvention paymentConvention() const;

        const Leg& fixedLeg() const;
        const Leg& floatingLeg() const;
        //@}

        //! \name Results
        //@{
        Real fixedLegBPS() const;
        Real fixedLegNPV() const;
        Rate fairRate() const;

        Real floatingLegBPS() const;
        Real floatingLegNPV() const;
        Spread fairSpread() const;
        //@}
        // other
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;
        virtual void setupFloatingArguments(arguments* args) const = 0;
        Type type_;
        std::vector<Real> fixedNominals_;
        Schedule fixedSchedule_;
        Rate fixedRate_;
        DayCounter fixedDayCount_;
        std::vector<Real> floatingNominals_;
        Schedule floatingSchedule_;
        ext::shared_ptr<IborIndex> iborIndex_;
        Spread spread_;
        DayCounter floatingDayCount_;
        BusinessDayConvention paymentConvention_;
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;

        bool constantNominals_, sameNominals_;
    };


    //! %Arguments for simple swap calculation
    class FixedVsFloatingSwap::arguments : public Swap::arguments {
      public:
        arguments() : nominal(Null<Real>()) {}
        Type type = Receiver;
        Real nominal;

        std::vector<Real> fixedNominals;
        std::vector<Date> fixedResetDates;
        std::vector<Date> fixedPayDates;
        std::vector<Real> floatingNominals;
        std::vector<Time> floatingAccrualTimes;
        std::vector<Date> floatingResetDates;
        std::vector<Date> floatingFixingDates;
        std::vector<Date> floatingPayDates;

        std::vector<Real> fixedCoupons;
        std::vector<Spread> floatingSpreads;
        std::vector<Real> floatingCoupons;
        void validate() const override;
    };

    //! %Results from simple swap calculation
    class FixedVsFloatingSwap::results : public Swap::results {
      public:
        Rate fairRate;
        Spread fairSpread;
        void reset() override;
    };

    class FixedVsFloatingSwap::engine : public GenericEngine<FixedVsFloatingSwap::arguments,
                                                             FixedVsFloatingSwap::results> {};


    // inline definitions

    inline Swap::Type FixedVsFloatingSwap::type() const {
        return type_;
    }

    inline Real FixedVsFloatingSwap::nominal() const {
        QL_REQUIRE(constantNominals_, "nominal is not constant");
        return fixedNominals_[0];
    }

    inline const std::vector<Real>& FixedVsFloatingSwap::nominals() const {
        QL_REQUIRE(sameNominals_, "different nominals on fixed and floating leg");
        return fixedNominals_;
    }

    inline const std::vector<Real>& FixedVsFloatingSwap::fixedNominals() const {
        return fixedNominals_;
    }

    inline const Schedule& FixedVsFloatingSwap::fixedSchedule() const {
        return fixedSchedule_;
    }

    inline Rate FixedVsFloatingSwap::fixedRate() const {
        return fixedRate_;
    }

    inline const DayCounter& FixedVsFloatingSwap::fixedDayCount() const {
        return fixedDayCount_;
    }

    inline const std::vector<Real>& FixedVsFloatingSwap::floatingNominals() const {
        return floatingNominals_;
    }

    inline const Schedule& FixedVsFloatingSwap::floatingSchedule() const {
        return floatingSchedule_;
    }

    inline const ext::shared_ptr<IborIndex>& FixedVsFloatingSwap::iborIndex() const {
        return iborIndex_;
    }

    inline Spread FixedVsFloatingSwap::spread() const {
        return spread_;
    }

    inline const DayCounter& FixedVsFloatingSwap::floatingDayCount() const {
        return floatingDayCount_;
    }

    inline BusinessDayConvention FixedVsFloatingSwap::paymentConvention() const {
        return paymentConvention_;
    }

    inline const Leg& FixedVsFloatingSwap::fixedLeg() const {
        return legs_[0];
    }

    inline const Leg& FixedVsFloatingSwap::floatingLeg() const {
        return legs_[1];
    }

}

#endif
