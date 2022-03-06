/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009, 2011 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file cpiswap.hpp
 \brief zero-inflation-indexed-ratio-with-base swap
 */

#ifndef quantlib_zeroinflationswap_hpp
#define quantlib_zeroinflationswap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/cashflows/cpicoupon.hpp>

namespace QuantLib {

    class ZeroInflationIndex;

    //! zero-inflation-indexed swap,
    /*! fixed x zero-inflation, i.e. fixed x CPI(i'th fixing)/CPI(base)
        versus floating + spread

        Note that this does ony the inflation-vs-floating-leg.
        Extension to inflation-vs-fixed-leg.  is simple - just replace
        the floating leg with a fixed leg.

        Typically there are notional exchanges at the end: either
        inflated-notional vs notional; or just (inflated-notional -
        notional) vs zero.  The latter is perhaphs more typical.
        \warning Setting subtractInflationNominal to true means that
        the original inflation nominal is subtracted from both
        nominals before they are exchanged, even if they are
        different.

        This swap can mimic a ZCIIS where [(1+q)^n - 1] is exchanged
        against (cpi ratio - 1), by using differnt nominals on each
        leg and setting subtractInflationNominal to true.  ALSO -
        there must be just one date in each schedule.

        The two legs can have different schedules, fixing (days vs
        lag), settlement, and roll conventions.  N.B. accrual
        adjustment periods are already in the schedules.  Trade date
        and swap settlement date are outside the scope of the
        instrument.
    */
    class CPISwap : public Swap {
      public:
        class arguments;
        class results;
        class engine;

        /*! In this swap, the type (Payer or Receiver) refers to the floating leg. */
        CPISwap(Type type,
                Real nominal,
                bool subtractInflationNominal,
                // float+spread leg
                Spread spread,
                DayCounter floatDayCount,
                Schedule floatSchedule,
                const BusinessDayConvention& floatRoll,
                Natural fixingDays,
                ext::shared_ptr<IborIndex> floatIndex,
                // fixed x inflation leg
                Rate fixedRate,
                Real baseCPI,
                DayCounter fixedDayCount,
                Schedule fixedSchedule,
                const BusinessDayConvention& fixedRoll,
                const Period& observationLag,
                ext::shared_ptr<ZeroInflationIndex> fixedIndex,
                CPI::InterpolationType observationInterpolation = CPI::AsIndex,
                Real inflationNominal = Null<Real>());

        // results
        // float+spread
        virtual Real floatLegNPV() const;
        virtual Spread fairSpread() const;
        // fixed rate x inflation
        virtual Real fixedLegNPV() const;
        virtual Rate fairRate() const;

        // inspectors
        virtual Type type() const;
        virtual Real nominal() const;
        virtual bool subtractInflationNominal() const;

        // float+spread
        virtual Spread spread() const;
        virtual const DayCounter& floatDayCount() const;
        virtual const Schedule& floatSchedule() const;
        virtual const BusinessDayConvention& floatPaymentRoll() const;
        virtual Natural fixingDays() const;
        virtual const ext::shared_ptr<IborIndex>& floatIndex() const;

        // fixed rate x inflation
        virtual Rate fixedRate() const;
        virtual Real baseCPI() const;
        virtual const DayCounter& fixedDayCount() const;
        virtual const Schedule& fixedSchedule() const;
        virtual const BusinessDayConvention& fixedPaymentRoll() const;
        virtual Period observationLag() const;
        virtual const ext::shared_ptr<ZeroInflationIndex>& fixedIndex() const;
        virtual CPI::InterpolationType observationInterpolation() const;
        virtual Real inflationNominal() const;

        // legs
        virtual const Leg& cpiLeg() const;
        virtual const Leg& floatLeg() const;

        // other
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;

      private:
        void setupExpired() const override;

        Type type_;
        Real nominal_;
        bool subtractInflationNominal_;

        // float+spread leg
        Spread spread_;
        DayCounter floatDayCount_;
        Schedule floatSchedule_;
        BusinessDayConvention floatPaymentRoll_;
        Natural fixingDays_;
        ext::shared_ptr<IborIndex> floatIndex_;

        // fixed x inflation leg
        Rate fixedRate_;
        Real baseCPI_;
        DayCounter fixedDayCount_;
        Schedule fixedSchedule_;
        BusinessDayConvention fixedPaymentRoll_;
        ext::shared_ptr<ZeroInflationIndex> fixedIndex_;
        Period observationLag_;
        CPI::InterpolationType observationInterpolation_;
        Real inflationNominal_;
        // results
        mutable Spread fairSpread_;
        mutable Rate fairRate_;

    };


    //! %Arguments for swap calculation
    class CPISwap::arguments : public Swap::arguments {
    public:
      arguments() : nominal(Null<Real>()) {}
      Type type = Receiver;
      Real nominal;

      void validate() const override;
    };

    //! %Results from swap calculation
    class CPISwap::results : public Swap::results {
    public:
        Rate fairRate;
        Spread fairSpread;
        void reset() override;
    };

    class CPISwap::engine : public GenericEngine<CPISwap::arguments,
                                                 CPISwap::results> {};


    // inline definitions

    // inspectors
    inline  Swap::Type CPISwap::type() const { return type_; }
    inline  Real CPISwap::nominal() const { return nominal_; }
    inline  bool CPISwap::subtractInflationNominal() const { return subtractInflationNominal_; }

    // float+spread
    inline Spread CPISwap::spread() const { return spread_; }
    inline const DayCounter& CPISwap::floatDayCount() const { return floatDayCount_; }
    inline const Schedule& CPISwap::floatSchedule() const { return floatSchedule_; }
    inline const BusinessDayConvention& CPISwap::floatPaymentRoll() const { return floatPaymentRoll_; }
    inline Natural CPISwap::fixingDays() const { return fixingDays_; }
    inline const ext::shared_ptr<IborIndex>& CPISwap::floatIndex() const { return floatIndex_; }

    // fixed rate x inflation
    inline Rate CPISwap::fixedRate() const { return fixedRate_; }
    inline Real CPISwap::baseCPI() const { return baseCPI_; }
    inline const DayCounter& CPISwap::fixedDayCount() const { return fixedDayCount_; }
    inline const Schedule& CPISwap::fixedSchedule() const { return fixedSchedule_; }
    inline const BusinessDayConvention& CPISwap::fixedPaymentRoll() const { return fixedPaymentRoll_; }
    inline Period CPISwap::observationLag() const { return observationLag_; }
    inline const ext::shared_ptr<ZeroInflationIndex>& CPISwap::fixedIndex() const { return fixedIndex_; }
    inline CPI::InterpolationType CPISwap::observationInterpolation() const { return observationInterpolation_; }
    inline Real CPISwap::inflationNominal() const { return inflationNominal_; }

    inline const Leg& CPISwap::cpiLeg() const {//inflation indexed
        return legs_[0];
    }

    inline const Leg& CPISwap::floatLeg() const {
        return legs_[1];
    }

}

#endif



#ifndef id_8d95064ba65c770211c44c5173bda8ad
#define id_8d95064ba65c770211c44c5173bda8ad
inline bool test_8d95064ba65c770211c44c5173bda8ad(int* i) { return i != 0; }
#endif
