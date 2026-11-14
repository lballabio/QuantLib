/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file tradingtimeyieldtermstructure.hpp
    \brief A yield curve on the trading-time axis
*/

#ifndef quantlib_trading_time_yield_term_structure_hpp
#define quantlib_trading_time_yield_term_structure_hpp

#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! A yield curve whose times are trading times
    /*! The discount factor at every date is that of the given curve: only
        the times change, measured in trading time (see
        TradingTimeDayCounter), so that processes and engines built on this
        curve run on the trading-time clock.  Between dates, the discount
        factor at a trading time is the curve's at the corresponding
        calendar time (TradingTimeTermStructure::calendarTime()), which the
        curve's own day counter measures linearly within each day.

        Trading time always runs, so its times map to calendar times
        one-to-one.  The curve and the trading time must have the same
        reference date, which is checked when the curve is built and
        whenever either notifies.
    */
    class TradingTimeYieldTermStructure : public YieldTermStructure {
      public:
        TradingTimeYieldTermStructure(Handle<YieldTermStructure> curve,
                                      Handle<TradingTimeTermStructure> tradingTime);

        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const override { return curve_->referenceDate(); }
        Calendar calendar() const override { return curve_->calendar(); }
        Natural settlementDays() const override { return curve_->settlementDays(); }
        Date maxDate() const override { return curve_->maxDate(); }
        //@}

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! The curve on the calendar-time axis.
        const Handle<YieldTermStructure>& calendarTimeCurve() const { return curve_; }
        const Handle<TradingTimeTermStructure>& tradingTime() const { return tradingTime_; }

      protected:
        DiscountFactor discountImpl(Time tau) const override;

      private:
        void checkTradingTime() const;

        Handle<YieldTermStructure> curve_;
        Handle<TradingTimeTermStructure> tradingTime_;
        mutable bool checked_ = false;
    };

}

#endif
