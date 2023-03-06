/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

/*! \file equitytotalreturnswap.hpp
 \brief Equity total return swap
 */

#ifndef quantlib_equitytotalreturnswap_hpp
#define quantlib_equitytotalreturnswap_hpp

#include <ql/instruments/swap.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {
    class InterestRateIndex;
    class IborIndex;
    class OvernightIndex;
    class EquityIndex;

    //! Equity total return swap
    /*! It exchanges a total return of an equity index for a set of
        floating cash flows linked to either Ibor or overnight index.
        The equity leg future value (FV) is:
        \f[
        FV^{equity} = N \left[ \frac{I(t, T_{M})}{I(T_{0})} -1 \right],
        \f]
        where \f$ N \f$ is the swap notional, \f$ I(T_{0}) \f$ is the
        value of the equity index on the start date and \f$ I(t, T_{M}) \f$
        is the value of the equity index at maturity.
        The floating leg payments are linked to either an Ibor or
        an overnight index. In case of an overnight index the interest
        rate fixings are compounded over the accrual period.

        Swap type (payer of receiver) refers to the equity leg.
    */
    class EquityTotalReturnSwap : public Swap {
      public:
        EquityTotalReturnSwap(Type type,
                              Real nominal,
                              Schedule schedule,
                              ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<IborIndex>& interestRateIndex,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0);

        EquityTotalReturnSwap(Type type,
                              Real nominal,
                              Schedule schedule,
                              ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<OvernightIndex>& interestRateIndex,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0);

        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        Real nominal() const { return nominal_; }
        const ext::shared_ptr<EquityIndex>& equityIndex() const { return equityIndex_; } 
        const ext::shared_ptr<InterestRateIndex>& interestRateIndex() const { return interestRateIndex_; }
        const Schedule& schedule() const { return schedule_; }
        const DayCounter& dayCounter() const { return dayCounter_; }
        Rate margin() const { return margin_; }
        Real gearing() const { return gearing_; }
        const Calendar& paymentCalendar() const { return paymentCalendar_; }
        BusinessDayConvention paymentConvention() const { return paymentConvention_; }
        Natural paymentDelay() const { return paymentDelay_; }

        const Leg& equityLeg() const;
        const Leg& interestRateLeg() const;
        //@}

        //! \name Results
        //@{
        Real equityLegNPV() const;
        Real interestRateLegNPV() const;
        Real fairMargin() const;
        //@}

      private:
        EquityTotalReturnSwap(ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<InterestRateIndex>& interestRateIndex,
                              Type type,
                              Real nominal,
                              Schedule schedule,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0);

        ext::shared_ptr<EquityIndex> equityIndex_;
        ext::shared_ptr<InterestRateIndex> interestRateIndex_;
        Type type_;
        Real nominal_;
        Schedule schedule_;
        DayCounter dayCounter_;
        Rate margin_;
        Real gearing_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentConvention_;
        Natural paymentDelay_;
    };
}

#endif