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

/*! \file fxtradingtimemarket.hpp
    \brief An FX market on the trading-time clock, for local and stochastic local vol
*/

#ifndef quantlib_fx_trading_time_market_hpp
#define quantlib_fx_trading_time_market_hpp

#include <ql/experimental/fxslv/conventions/fxsettlementconvention.hpp>
#include <ql/experimental/fxslv/tradingtime/tradingtimedaycounter.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    //! An FX market on the trading-time clock
    /*! Local vol, and the variance and leverage of stochastic local vol,
        follow the variance surface: in calendar time they drop over
        weekends and holidays and spike at events, day by day, which
        finite-difference calibrations sample poorly unless their steps
        fall on midnights.  On the trading-time clock of the surface the
        same market is smooth.  This builds the market on that clock:

        - curves with the same discount factor at every date, whose times
          are trading times (TradingTimeYieldTermStructure);
        - the variance surface with the same total variance at every date,
          whose implied vols are per year of trading time
          (FxTradingTimeVarianceSurface);
        - from them, the settlement-forward foreign curve and the Dupire
          local vol, in trading time, and the local-vol process
          (makeFxLocalVolProcess()).

        Instruments keep their calendar dates, and everything that turns
        dates into times does so through these curves, so models
        calibrated and engines pricing on them run in trading time:
        pass them to FxSLVPricingContext, or price with the local-vol
        process.  Prices are those of the calendar-time market for
        vanillas, and a time change leaves continuously monitored
        barriers unchanged; dates are mapped one-to-one.

        Model parameters calibrated on this market are per year of trading
        time, e.g. Heston's kappa, theta and sigma.

        \todo Convert calibrated parameters to calendar-time equivalents
              for reporting, e.g. with the average trading time per
              calendar year over the calibration horizon.

    */
    class FxTradingTimeMarket {
      public:
        /*! \param spot             spot rate, domestic per unit of foreign.
            \param foreignCurve     foreign discount curve, unadjusted, in calendar time.
            \param domesticCurve    domestic discount curve, in calendar time.
            \param settlement       spot lag and calendar of the pair.
            \param volSurface       a variance surface interpolated in trading
                                    time, e.g. an fxVarianceSurface; its trading
                                    time is the market's clock.
            \param localVolFallback volatility to use where the local vol is not
                                    defined, per year of trading time; none by
                                    default, so that pricing fails there.
        */
        FxTradingTimeMarket(Handle<Quote> spot,
                            const Handle<YieldTermStructure>& foreignCurve,
                            const Handle<YieldTermStructure>& domesticCurve,
                            const FxSettleConvention& settlement,
                            const ext::shared_ptr<BlackVolTermStructure>& volSurface,
                            Volatility localVolFallback = Null<Volatility>());

        //! \name The market on the trading-time clock
        //@{
        const Handle<Quote>& spot() const { return spot_; }
        const Handle<TradingTimeTermStructure>& tradingTime() const { return tradingTime_; }
        //! The clock: year fractions are trading times.
        const DayCounter& dayCounter() const { return dayCounter_; }
        const Handle<YieldTermStructure>& domesticCurve() const { return domesticCurve_; }
        //! The foreign curve, unadjusted.
        const Handle<YieldTermStructure>& foreignCurve() const { return foreignCurve_; }
        //! The foreign curve giving processes the settlement forward, see FxSettlementForeignCurve.
        const Handle<YieldTermStructure>& settlementForeignCurve() const {
            return settlementForeignCurve_;
        }
        const Handle<BlackVolTermStructure>& volSurface() const { return volSurface_; }
        const Handle<LocalVolTermStructure>& localVol() const { return localVol_; }
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& localVolProcess() const {
            return localVolProcess_;
        }
        //@}

      private:
        Handle<Quote> spot_;
        Handle<TradingTimeTermStructure> tradingTime_;
        DayCounter dayCounter_;
        Handle<YieldTermStructure> domesticCurve_, foreignCurve_, settlementForeignCurve_;
        Handle<BlackVolTermStructure> volSurface_;
        Handle<LocalVolTermStructure> localVol_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> localVolProcess_;
    };

}

#endif
