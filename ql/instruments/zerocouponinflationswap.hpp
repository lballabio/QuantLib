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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
 */

/*! \file zerocouponinflationswap.hpp
 \brief Zero-coupon inflation-indexed swap
 */

#ifndef quantlib_xxxzciis_hpp
#define quantlib_xxxzciis_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>



namespace QuantLib {
    class ZeroInflationIndex;

    //! Zero-coupon inflation-indexed swap
    /*! Quoted as a fixed rate \f$ K \f$.  At start:
        \f[
        P_n(0,T) N [(1+K)^{T}-1] =
        P_n(0,T) N \left[ \frac{I(T)}{I(0)} -1 \right]
        \f]
        where \f$ T \f$ is the maturity time, \f$ P_n(0,t) \f$ is the
        nominal discount factor at time \f$ t \f$, \f$ N \f$ is the
        notional, and \f$ I(t) \f$ is the inflation index value at
        time \f$ t \f$.

        This inherits from swap and has two very simple legs: a fixed
        leg, from the quote (K); and an indexed leg.  At maturity the
        two single cashflows are swapped.  These are the notional
        versus the inflation-indexed notional Because the coupons are
        zero there are no accruals (and no coupons).

        Inflation is generally available on every day, including
        holidays and weekends.  Hence there is a variable to state
        whether the observe/fix dates for inflation are adjusted or
        not.  The default is not to adjust.

        A zero inflation swap is a simple enough instrument that the
        standard discounting pricing engine that works for a vanilla
        swap also works.

        \note we do not need Schedules on the legs because they use
              one or two dates only per leg.
    */
    class ZeroCouponInflationSwap : public Swap {
      public:
        enum Type { Receiver = -1, Payer = 1 };
        class arguments;
        class engine;
        ZeroCouponInflationSwap(Type type,
                                Real nominal,
                                const Date& startDate,   // start date of contract (only)
                                const Date& maturity,    // this is pre-adjustment!
                                const Calendar& fixCalendar,
                                BusinessDayConvention fixConvention,
                                const DayCounter& dayCounter,
                                Rate fixedRate,
                                const ext::shared_ptr<ZeroInflationIndex> &infIndex,
                                const Period& observationLag,
                                bool adjustInfObsDates = false,
                                const Calendar& infCalendar = Calendar(),
                                BusinessDayConvention infConvention = BusinessDayConvention());

        //! \name Inspectors
        //@{
        //! "payer" or "receiver" refer to the inflation-indexed leg
        Type type() const { return type_; }
        Real nominal() const { return nominal_; }
        Date startDate() const { return startDate_; }
        Date maturityDate() const { return maturityDate_; }
        Calendar fixedCalendar() const { return fixCalendar_; }
        BusinessDayConvention fixedConvention() const {
            return fixConvention_;
        }
        DayCounter dayCounter() const { return dayCounter_; }
        //! \f$ K \f$ in the above formula.
        Rate fixedRate() const { return fixedRate_; }
        ext::shared_ptr<ZeroInflationIndex> inflationIndex() const {
            return infIndex_;
        }
        Period observationLag() const { return observationLag_; }
        bool adjustObservationDates() const { return adjustInfObsDates_; }
        Calendar inflationCalendar() const { return infCalendar_; }
        BusinessDayConvention inflationConvention() const {
            return infConvention_;
        }
        //! just one cashflow (that is not a coupon) in each leg
        const Leg& fixedLeg() const;
        //! just one cashflow (that is not a coupon) in each leg
        const Leg& inflationLeg() const;
        //@}

        //! \name Instrument interface
        //@{
        void setupArguments(PricingEngine::arguments*) const;
        void fetchResults(const PricingEngine::results* r) const;
        //@}

        //! \name Results
        //@{
        Real fixedLegNPV() const;
        Real inflationLegNPV() const;
        Real fairRate() const;
        //@}

      protected:
        Type type_;
        Real nominal_;
        Date startDate_, maturityDate_;
        Calendar fixCalendar_;
        BusinessDayConvention fixConvention_;
        Rate fixedRate_;
        ext::shared_ptr<ZeroInflationIndex> infIndex_;
        Period observationLag_;
        bool adjustInfObsDates_;
        Calendar infCalendar_;
        BusinessDayConvention infConvention_;
        DayCounter dayCounter_;
        Date baseDate_, obsDate_;
    };


    class ZeroCouponInflationSwap::arguments : public Swap::arguments {
      public:
        Rate fixedRate;
        void validate() const;
    };


    class ZeroCouponInflationSwap::engine
    : public GenericEngine<ZeroCouponInflationSwap::arguments,
    ZeroCouponInflationSwap::results> {};

}


#endif
