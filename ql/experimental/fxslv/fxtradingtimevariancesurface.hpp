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

/*! \file fxtradingtimevariancesurface.hpp
    \brief An FX variance surface on the trading-time axis
*/

#ifndef quantlib_fx_trading_time_variance_surface_hpp
#define quantlib_fx_trading_time_variance_surface_hpp

#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/patterns/observable.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>

namespace QuantLib {

    //! A variance surface interpolated in trading time, which it can be read in
    /*! Implemented by fxVarianceSurface, so that its trading-time view
        need not depend on the smile section it is built from.
    */
    class FxTradingTimeVariance : public virtual Observable {
      public:
        virtual ~FxTradingTimeVariance() = default;
        //! Total variance at a trading time from the reference date.
        virtual Real blackVarianceInTradingTime(Time tradingTime, Real strike) const = 0;
        //! The trading time the variance is interpolated in.
        virtual const Handle<TradingTimeTermStructure>& tradingTimeStructure() const = 0;
    };

    //! An FX variance surface whose times are trading times
    /*! The total variance at every date and strike is that of the given
        surface: only the times change, measured in trading time (see
        TradingTimeDayCounter), so the implied vol of an expiry is per
        year of trading time, \f$ \sigma_\tau = \sqrt{w / \tau} \f$.
        Between dates the variance is read in trading time, in which the
        surface is interpolated, so that it grows smoothly through an
        event, which takes no calendar time.  Local vol (Dupire) and models
        calibrated on this surface are on the trading-time clock: local
        vol stays smooth over weekends and events.

    */
    class FxTradingTimeVarianceSurface : public BlackVarianceTermStructure {
      public:
        /*! \param surface a variance surface interpolated in trading time,
                           e.g. an fxVarianceSurface.
        */
        explicit FxTradingTimeVarianceSurface(ext::shared_ptr<BlackVolTermStructure> surface);

        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const override { return surface_->referenceDate(); }
        Calendar calendar() const override { return surface_->calendar(); }
        Natural settlementDays() const override { return surface_->settlementDays(); }
        Date maxDate() const override { return surface_->maxDate(); }
        //@}

        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return surface_->minStrike(); }
        Real maxStrike() const override { return surface_->maxStrike(); }
        //@}

        //! The surface on the calendar-time axis.
        const ext::shared_ptr<BlackVolTermStructure>& calendarTimeSurface() const {
            return surface_;
        }

      protected:
        Real blackVarianceImpl(Time tau, Real strike) const override;

      private:
        ext::shared_ptr<BlackVolTermStructure> surface_;
        ext::shared_ptr<FxTradingTimeVariance> variance_;
    };

}

#endif
