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

/*! \file fxslvpricingcontext.hpp
    \brief Orchestrates calibration and pricing for an FX SLV model
*/

#ifndef quantlib_fx_slv_pricing_context_hpp
#define quantlib_fx_slv_pricing_context_hpp

#include <ql/experimental/fxslv/hestoncalibrator.hpp>
#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>
#include <ql/handle.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengine.hpp>
#include <ql/quote.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/calendar.hpp>
#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Grid dimensions used by the SLV finite-difference pricing engines.
    struct FdmGridConfig {
        Size tGrid        = 100; //!< number of time steps
        Size xGrid        = 100; //!< log-spot grid points
        Size vGrid        = 50;  //!< variance grid points
        Size dampingSteps = 0;   //!< Rannacher smoothing steps at maturity
    };

    //! Orchestrator for calibrating and pricing under an FX Stochastic Local Vol model.
    /*!
        \c FxSLVPricingContext bundles together all the market data, calibrated
        models, and leverage function needed to price and risk-manage a book of
        FX vanilla and barrier options under a Stochastic Local Volatility (SLV)
        model.

        Typical usage:
        \code
        // 1. Assemble market data.
        FxSLVPricingContext ctx(spot, domesticTS, foreignTS, volSurface,
                                localVolSurface, calendar);

        // 2. Define calibration pillars (expiry, absolute strike).
        std::vector<StochVolCalibrator::Pillar> pillars = {
            {3*Months, 1.08}, {6*Months, 1.10}, {1*Years, 1.12}, ...
        };

        // 3. Calibrate the Heston model, then the leverage function.
        HestonCalibrator hestonCal;
        HestonSLVLeverageCalibrator lvCal(Handle<HestonModel>(...));
        ctx.calibrate(hestonCal, lvCal, pillars, lastExpiry);

        // 4. Price instruments.
        myVanilla.setPricingEngine(ctx.vanillaEngine());
        myBarrier.setPricingEngine(ctx.barrierEngine());
        \endcode

        \ingroup experimental
    */
    class FxSLVPricingContext {
      public:
        //! \name Construction
        //@{
        /*!
            \param spot          FX spot rate quote (units: domestic per foreign)
            \param domesticTS    domestic risk-free yield term structure
            \param foreignTS     foreign risk-free (or dividend) yield term structure
            \param volSurface    market Black implied volatility surface
            \param localVol      Dupire local volatility surface derived from
                                 \p volSurface (supplied externally so the user
                                 can choose their own interpolation scheme)
            \param calendar      calendar used for expiry date generation
        */
        FxSLVPricingContext(Handle<Quote>                    spot,
                            Handle<YieldTermStructure>       domesticTS,
                            Handle<YieldTermStructure>       foreignTS,
                            Handle<BlackVolTermStructure>    volSurface,
                            Handle<LocalVolTermStructure>    localVol,
                            Calendar                         calendar);
        //@}

        //! \name Calibration
        //@{
        /*!
            Runs the two-step calibration:
              1. Heston stochastic vol calibration via \p hestonCal
              2. Leverage function calibration via \p leverageCal

            After this call the context is ready to produce pricing engines.

            \param hestonCal     stochastic vol calibrator (Heston or a subclass)
            \param leverageCal   SLV leverage function calibrator
            \param pillars       (expiry, strike) calibration instruments
            \param leverageEndDate end date for the leverage function PDE grid;
                                 typically the maturity of the longest instrument
                                 in the book
            \param mandatoryDates additional time grid dates for the leverage PDE
        */
        void calibrate(StochVolCalibrator&              hestonCal,
                       SLVLeverageCalibrator&           leverageCal,
                       const std::vector<StochVolCalibrator::Pillar>& pillars,
                       const Date&                      leverageEndDate,
                       const std::vector<Date>&         mandatoryDates = {});
        //@}

        //! \name Pricing engine factories
        //@{
        //! Finite-difference Heston SLV engine for European / American vanillas.
        ext::shared_ptr<PricingEngine> vanillaEngine(
            const FdmGridConfig& grid    = FdmGridConfig{},
            const FdmSchemeDesc& scheme  = FdmSchemeDesc::Hundsdorfer()) const;

        //! Finite-difference Heston SLV engine for single-barrier options.
        ext::shared_ptr<PricingEngine> barrierEngine(
            const FdmGridConfig& grid    = FdmGridConfig{},
            const FdmSchemeDesc& scheme  = FdmSchemeDesc::Hundsdorfer()) const;

        //! Finite-difference Heston SLV engine for double-barrier options.
        ext::shared_ptr<PricingEngine> doubleBarrierEngine(
            const FdmGridConfig& grid    = FdmGridConfig{},
            const FdmSchemeDesc& scheme  = FdmSchemeDesc::Hundsdorfer()) const;
        //@}

        //! \name Model accessors
        //@{
        //! The calibrated Heston model.  Throws if calibrate() not yet called.
        ext::shared_ptr<HestonModel> hestonModel() const;

        //! The calibrated leverage function L(t,S).  Throws if calibrate() not yet called.
        ext::shared_ptr<LocalVolTermStructure> leverageFunction() const;

        //! Per-pillar implied-vol calibration errors from the Heston fit.
        std::vector<Real> hestonCalibrationErrors() const;

        //! RMSE of the Heston fit across all calibration pillars.
        Real hestonRmse() const;
        //@}

        //! \name Market data accessors
        //@{
        const Handle<Quote>&                 spot()       const { return spot_; }
        const Handle<YieldTermStructure>&    domesticTS() const { return domesticTS_; }
        const Handle<YieldTermStructure>&    foreignTS()  const { return foreignTS_; }
        const Handle<BlackVolTermStructure>& volSurface() const { return volSurface_; }
        const Handle<LocalVolTermStructure>& localVol()   const { return localVol_; }
        //@}

      private:
        Handle<Quote>                 spot_;
        Handle<YieldTermStructure>    domesticTS_;
        Handle<YieldTermStructure>    foreignTS_;
        Handle<BlackVolTermStructure> volSurface_;
        Handle<LocalVolTermStructure> localVol_;
        Calendar                      calendar_;

        ext::shared_ptr<HestonModel>          hestonModel_;
        ext::shared_ptr<LocalVolTermStructure> leverageFunction_;
        std::vector<Real>                      hestonErrors_;
        Real                                   hestonRmse_ = 0.0;
    };

}

#endif
