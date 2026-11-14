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

/*! \file tradingtimedaycounter.hpp
    \brief Day counter measuring time in trading time
*/

#ifndef quantlib_trading_time_day_counter_hpp
#define quantlib_trading_time_day_counter_hpp

#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/handle.hpp>
#include <ql/time/daycounter.hpp>

namespace QuantLib {

    //! Day counter whose year fractions are trading times
    /*! The year fraction between two dates is the trading time between
        them (see TradingTimeTermStructure), so a term structure, process
        or engine taking its times from this day counter runs on the
        trading-time clock.  The day count is in calendar days.

        Dates must not precede the trading time's reference date.

        \warning A day counter cannot notify: a term structure using this
                 one must also register with the trading time to see its
                 event weights change.  Day counters compare by name, so
                 two of these compare equal whatever their trading times.
    */
    class TradingTimeDayCounter : public DayCounter {
      private:
        class Impl final : public DayCounter::Impl {
          public:
            explicit Impl(Handle<TradingTimeTermStructure> tradingTime);
            std::string name() const override { return "Trading time"; }
            Time yearFraction(const Date& d1,
                              const Date& d2,
                              const Date&,
                              const Date&) const override;

          private:
            Handle<TradingTimeTermStructure> tradingTime_;
        };

      public:
        explicit TradingTimeDayCounter(const Handle<TradingTimeTermStructure>& tradingTime)
        : DayCounter(ext::make_shared<TradingTimeDayCounter::Impl>(tradingTime)) {}
    };

}

#endif
