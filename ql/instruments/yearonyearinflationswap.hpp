/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

/*! \file yearonyearinflationswap.hpp
 \brief Year-on-year inflation-indexed swap
 */

#ifndef quantlib_yyiis_hpp
#define quantlib_yyiis_hpp

#include <ql/instruments/swap.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {

    //! Year-on-year inflation-indexed swap
    /*! Quoted as a fixed rate \f$ K \f$.  At start:
        \f[
        \sum_{i=1}^{M} P_n(0,t_i) N K =
        \sum_{i=1}^{M} P_n(0,t_i) N \left[ \frac{I(t_i)}{I(t_i-1)} - 1 \right]
        \f]
        where \f$ t_M \f$ is the maturity time, \f$ P_n(0,t) \f$ is the
        nominal discount factor at time \f$ t \f$, \f$ N \f$ is the
        notional, and \f$ I(t) \f$ is the inflation index value at
        time \f$ t \f$.
    */
    class YearOnYearInflationSwap : public Swap {
    public:
        class arguments;
        class results;
        class engine;
        YearOnYearInflationSwap(
            Type type,
            Real nominal,
            Schedule fixedSchedule,
            Rate fixedRate,
            DayCounter fixedDayCount,
            Schedule yoySchedule,
            ext::shared_ptr<YoYInflationIndex> yoyIndex,
            const Period& observationLag,
            CPI::InterpolationType interpolation,
            Spread spread,
            DayCounter yoyDayCount,
            Calendar paymentCalendar, // inflation index does not have a calendar
            BusinessDayConvention paymentConvention = ModifiedFollowing);
        /*! \deprecated Use the overload that passes an interpolation type instead.
                        Deprecated in version 1.36.
        */
        [[deprecated("Use the overload that passes an interpolation type instead")]]
        YearOnYearInflationSwap(
            Type type,
            Real nominal,
            Schedule fixedSchedule,
            Rate fixedRate,
            DayCounter fixedDayCount,
            Schedule yoySchedule,
            ext::shared_ptr<YoYInflationIndex> yoyIndex,
            const Period& observationLag,
            Spread spread,
            DayCounter yoyDayCount,
            Calendar paymentCalendar, // inflation index does not have a calendar
            BusinessDayConvention paymentConvention = ModifiedFollowing);
        // results
        virtual Real fixedLegNPV() const;
        virtual Rate fairRate() const;

        virtual Real yoyLegNPV() const;
        virtual Spread fairSpread() const;
        // inspectors
        virtual Type type() const;
        virtual Real nominal() const;

        virtual const Schedule& fixedSchedule() const;
        virtual Rate fixedRate() const;
        virtual const DayCounter& fixedDayCount() const;

        virtual const Schedule& yoySchedule() const;
        virtual const ext::shared_ptr<YoYInflationIndex>& yoyInflationIndex() const;
        virtual Period observationLag() const { return observationLag_; }
        virtual Spread spread() const;
        virtual const DayCounter& yoyDayCount() const;

        virtual Calendar paymentCalendar() const { return paymentCalendar_; }
        virtual BusinessDayConvention paymentConvention() const;

        virtual const Leg& fixedLeg() const;
        virtual const Leg& yoyLeg() const;

        // other
        void setupArguments(PricingEngine::arguments* args) const override;
        void fetchResults(const PricingEngine::results*) const override;
        ~YearOnYearInflationSwap() override = default;

      private:
        void setupExpired() const override;
        Type type_;
        Real nominal_;
        Schedule fixedSchedule_;
        Rate fixedRate_;
        DayCounter fixedDayCount_;
        Schedule yoySchedule_;
        ext::shared_ptr<YoYInflationIndex> yoyIndex_;
        Period observationLag_;
        Spread spread_;
        DayCounter yoyDayCount_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentConvention_;
        // results
        mutable Rate fairRate_;
        mutable Spread fairSpread_;
    };


    //! %Arguments for YoY swap calculation
    class YearOnYearInflationSwap::arguments : public Swap::arguments {
    public:
      arguments() : nominal(Null<Real>()) {}
      Type type = Receiver;
      Real nominal;

      std::vector<Date> fixedResetDates;
      std::vector<Date> fixedPayDates;
      std::vector<Time> yoyAccrualTimes;
      std::vector<Date> yoyResetDates;
      std::vector<Date> yoyFixingDates;
      std::vector<Date> yoyPayDates;

      std::vector<Real> fixedCoupons;
      std::vector<Spread> yoySpreads;
      std::vector<Real> yoyCoupons;
      void validate() const override;
    };

    //! %Results from YoY swap calculation
    class YearOnYearInflationSwap::results : public Swap::results {
    public:
        Rate fairRate;
        Spread fairSpread;
        void reset() override;
    };

    class YearOnYearInflationSwap::engine : public GenericEngine<YearOnYearInflationSwap::arguments,
                                                                 YearOnYearInflationSwap::results> {};


    // inline definitions

    inline Swap::Type YearOnYearInflationSwap::type() const {
        return type_;
    }

    inline Real YearOnYearInflationSwap::nominal() const {
        return nominal_;
    }

    inline const Schedule& YearOnYearInflationSwap::fixedSchedule() const {
        return fixedSchedule_;
    }

    inline Rate YearOnYearInflationSwap::fixedRate() const {
        return fixedRate_;
    }

    inline const DayCounter& YearOnYearInflationSwap::fixedDayCount() const {
        return fixedDayCount_;
    }

    inline const Schedule& YearOnYearInflationSwap::yoySchedule() const {
        return yoySchedule_;
    }

    inline const ext::shared_ptr<YoYInflationIndex>& YearOnYearInflationSwap::yoyInflationIndex() const {
        return yoyIndex_;
    }

    inline Spread YearOnYearInflationSwap::spread() const {
        return spread_;
    }

    inline const DayCounter& YearOnYearInflationSwap::yoyDayCount() const {
        return yoyDayCount_;
    }

    inline BusinessDayConvention YearOnYearInflationSwap::paymentConvention() const {
        return paymentConvention_;
    }

    inline const Leg& YearOnYearInflationSwap::fixedLeg() const {
        return legs_[0];
    }

    inline const Leg& YearOnYearInflationSwap::yoyLeg() const {
        return legs_[1];
    }

}

#endif

