/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file fxlocalvolsurface.hpp
    \brief Dupire local volatility allowing periods without variance, and FX
           local-vol processes built on it
*/

#ifndef quantlib_fx_local_vol_surface_hpp
#define quantlib_fx_local_vol_surface_hpp

#include <ql/experimental/fxslv/fxsettlementconvention.hpp>
#include <ql/handle.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/quote.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! Dupire local volatility allowing periods without variance
    /*! The local variance is derived from the total Black variance
        \f$ w(y, t) \f$ at log-moneyness \f$ y = \ln(K/F(t)) \f$ as in
        QuantLib's LocalVolSurface (Gatheral's form),
        \f[
            \sigma^2(K, t) = \frac{\partial w / \partial t}
            {1 - \frac{y}{w}\frac{\partial w}{\partial y}
             + \frac{1}{4}\left(-\frac{1}{4} - \frac{1}{w} + \frac{y^2}{w^2}\right)
               \left(\frac{\partial w}{\partial y}\right)^2
             + \frac{1}{2}\frac{\partial^2 w}{\partial y^2}},
        \f]
        with derivatives by finite differences, but it allows periods
        in which no variance accrues, such as weekends or holidays in
        trading time:
        - a change of \f$ w \f$ over a time step within a relative
          tolerance, either way, is rounding, and is taken as no change;
        - where \f$ w \f$ does not change, the local volatility is zero
          (or the floor, if one is given);
        - a larger decrease is calendar arbitrage, and a non-positive
          denominator is butterfly arbitrage; either fails, saying so.

        A fallback volatility may be given explicitly, to be used where
        the local volatility is not defined instead of failing; the
        number of times it was used is counted, see fallbacks().  Without
        one, which is the default, such failures propagate.

        The forward is taken from the curves, as by the Black-Scholes
        process; for an FX variance surface, pass an
        FxSettlementForeignCurve as the foreign curve so that the two
        agree.  makeFxLocalVolProcess() does this wiring, and is the
        preferred way to build an FX local-vol process.
    */
    class FxLocalVolSurface : public LocalVolTermStructure {
      public:
        /*! \param tolerance    relative decrease of the total variance
                                over a time step taken as rounding.
            \param minLocalVol  floor of the local volatility, for engines
                                that cannot cope with zero diffusion; a
                                numerical device, zero by default.
            \param fallbackVol  volatility to use where the local volatility
                                is not defined, e.g. on arbitrage; none by
                                default, so that failures propagate.
        */
        FxLocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                          Handle<YieldTermStructure> riskFreeTS,
                          Handle<YieldTermStructure> dividendTS,
                          Handle<Quote> underlying,
                          Real tolerance = 1.0e-7,
                          Volatility minLocalVol = 0.0,
                          Volatility fallbackVol = Null<Volatility>());

        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const override { return blackTS_->referenceDate(); }
        DayCounter dayCounter() const override { return blackTS_->dayCounter(); }
        Date maxDate() const override { return blackTS_->maxDate(); }
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return blackTS_->minStrike(); }
        Real maxStrike() const override { return blackTS_->maxStrike(); }
        //@}

        //! Number of times the fallback volatility was used.
        Size fallbacks() const { return fallbacks_; }

      protected:
        Volatility localVolImpl(Time t, Real underlyingLevel) const override;

      private:
        Volatility dupireVol(Time t, Real underlyingLevel) const;
        // the variance at a later time, or the earlier one where they differ only by rounding;
        // fails if it decreases by more
        Real laterVariance(Real earlier, Real later, Real strike, Time t1, Time t2) const;

        Handle<BlackVolTermStructure> blackTS_;
        Handle<YieldTermStructure> riskFreeTS_, dividendTS_;
        Handle<Quote> underlying_;
        Real tolerance_;
        Volatility minLocalVol_;
        Volatility fallbackVol_;
        mutable Size fallbacks_ = 0;
    };


    //! Black-Scholes process for FX local-vol pricing, consistently wired
    /*! Pricing under local vol needs one forward in three places: the
        variance surface's moneyness, Dupire's extraction of the local
        vol, and the drift with which the engines move spot.  QuantLib's
        engines and Dupire take the forward from the process's curves,
        as \f$ S\,P_f(t)/P_d(t) \f$ from today to the expiry, while an
        FX option's forward runs from the spot date to its delivery date.
        This function builds the process so that all three agree:

        - its foreign (dividend) curve is an FxSettlementForeignCurve on
          the given curves, giving the settlement forward of each expiry;
        - its risk-free curve is the domestic curve;
        - its Black vol is the given surface;
        - its local vol is an FxLocalVolSurface on that surface and those
          curves, zero where no variance accrues and failing on arbitrage
          unless a fallback is given.

        The variance surface should be built on the same, unadjusted,
        foreign and domestic curves and the same settlement convention;
        it computes settlement forwards itself.

        The pieces can be recovered from the process:
        dividendYield() is the settlement foreign curve, to pass wherever
        a foreign curve consistent with it is needed (e.g. to SLV
        calibration), and localVolatility() is the FxLocalVolSurface,
        whose fallbacks() counts the uses of the fallback vol.

        \param spot             spot rate, domestic per unit of foreign.
        \param foreignCurve     foreign discount curve, unadjusted.
        \param domesticCurve    domestic discount curve.
        \param settlement       spot lag and calendar of the pair.
        \param volSurface       Black vol surface, e.g. an FX variance surface.
        \param localVolFallback volatility to use where the local vol is not
                                defined; none by default, so that pricing
                                fails there.

        \warning Payoffs are still discounted to the expiry rather than to
                 the delivery date, and path-dependent products see trade
                 times, not value dates; these need FX-aware engines.
    */
    ext::shared_ptr<GeneralizedBlackScholesProcess>
    makeFxLocalVolProcess(const Handle<Quote>& spot,
                          const Handle<YieldTermStructure>& foreignCurve,
                          const Handle<YieldTermStructure>& domesticCurve,
                          const FxSettleConvention& settlement,
                          const Handle<BlackVolTermStructure>& volSurface,
                          Volatility localVolFallback = Null<Volatility>());

}

#endif
