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
        //! "payer" or "receiver" refer to the equity leg.
        Type type() const { return type_; }
        Real nominal() const { return nominal_; }
        const ext::shared_ptr<EquityIndex>& equityIndex() const { return equityIndex_; } 
        const ext::shared_ptr<InterestRateIndex>& interestRateIndex() const {
            return interestRateIndex_;
        }

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
        EquityTotalReturnSwap(Type type,
                              Real nominal,
                              Schedule schedule,
                              ext::shared_ptr<EquityIndex> equityIndex,
                              const ext::shared_ptr<InterestRateIndex>& interestRateIndex,
                              DayCounter dayCounter,
                              Rate margin,
                              Real gearing = 1.0,
                              Calendar paymentCalendar = Calendar(),
                              BusinessDayConvention paymentConvention = Unadjusted,
                              Natural paymentDelay = 0);

        Type type_;
        Real nominal_;
        Schedule schedule_;
        ext::shared_ptr<EquityIndex> equityIndex_;
        ext::shared_ptr<InterestRateIndex> interestRateIndex_;
        DayCounter dayCounter_;
        Rate margin_;
        Real gearing_;
        Calendar paymentCalendar_;
        BusinessDayConvention paymentConvention_;
        Natural paymentDelay_;
    };
}

#endif