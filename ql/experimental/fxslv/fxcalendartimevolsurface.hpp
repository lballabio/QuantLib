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

/*! \file fxcalendartimevolsurface.hpp
    \brief An FX variance surface seen on the calendar-time axis
*/

#ifndef quantlib_fx_calendar_time_vol_surface_hpp
#define quantlib_fx_calendar_time_vol_surface_hpp

#include <ql/experimental/fxslv/tradingtime/tradingtimetermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>

namespace QuantLib {

    class FxVolSurface;

    //! An FX variance surface seen on the calendar-time axis
    /*! The total variance at every date and strike is that of the given
        surface, whose axis is trading time; only the times change, here
        Actual/365 (Fixed) from the reference date, so that the implied
        vols are per year of calendar time, as quoted.

        For models still calibrated in calendar time, with curves in
        calendar time: the Heston and leverage calibrations of SLV.  Local
        vol on it drops over weekends and spikes at events, day by day,
        which local vol on the surface itself does not.

        \todo Remove once stochastic local vol runs in trading time.
    */
    class FxCalendarTimeVolSurface : public BlackVarianceTermStructure {
      public:
        /*! \param surface an FX variance surface (an FxVolSurface, e.g. one
                           built by makeFxVolSurface()).
        */
        explicit FxCalendarTimeVolSurface(ext::shared_ptr<BlackVolTermStructure> surface);

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

        //! The surface on its own, trading-time, axis.
        const ext::shared_ptr<BlackVolTermStructure>& tradingTimeSurface() const {
            return surface_;
        }

      protected:
        Real blackVarianceImpl(Time t, Real strike) const override;

      private:
        ext::shared_ptr<BlackVolTermStructure> surface_;
        Handle<TradingTimeTermStructure> clock_;
    };

}

#endif
