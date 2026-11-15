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

#include <ql/experimental/fxslv/fxpairmarket.hpp>
#include <ql/experimental/fxslv/stochvolcalibrator.hpp>
#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/pricingengine.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
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

    //! Calibration and pricing of an FX pair under stochastic local vol
    /*! Calibrates a Heston model, then the leverage function, on an FX
        pair's market and gives pricing engines for vanilla and barrier
        options under the calibrated SLV model.

        Everything runs on the market's trading-time clock: the Heston
        model on its trading-time curves (with the settlement forward), and
        the leverage function against the Dupire local vol of its surface,
        whose time axis is trading time.  In calendar time local vol drops
        over weekends and spikes at events, day by day, and the leverage
        calibration does not converge to it.  Heston parameters are
        therefore per year of trading time.  The surface's pillars, where
        local vol jumps, are dates of the leverage calibration's time steps.

        \code
        FxSLVPricingContext context(market);
        HestonCalibrator hestonCal(...);
        HestonSLVLeverageCalibrator leverageCal(...);
        context.calibrate(hestonCal, leverageCal, pillars, lastExpiry);
        vanilla.setPricingEngine(context.vanillaEngine());
        \endcode

        \ingroup experimental
    */
    class FxSLVPricingContext {
      public:
        /*! \param market           the pair's market.
            \param localVolFallback volatility to use where Dupire local vol
                                    is not defined; none by default, so that
                                    the calibration fails there.
        */
        explicit FxSLVPricingContext(FxPairMarket market,
                                     Volatility localVolFallback = Null<Volatility>());

        //! \name Calibration
        //@{
        /*! Calibrates the Heston model to the pillars, then the leverage
            function up to the end date; then the context gives engines.

            \param hestonCal       stochastic vol calibrator.
            \param leverageCal     leverage function calibrator.
            \param pillars         (tenor, strike) calibration instruments.
            \param leverageEndDate end of the leverage calibration, typically
                                   the last expiry of the book, at most the
                                   surface's last pillar.
            \param mandatoryDates  further dates for the leverage calibration's
                                   time steps, besides the surface's pillars.
        */
        void calibrate(StochVolCalibrator& hestonCal,
                       SLVLeverageCalibrator& leverageCal,
                       const std::vector<StochVolCalibrator::Pillar>& pillars,
                       const Date& leverageEndDate,
                       std::vector<Date> mandatoryDates = {});
        //@}

        //! The same calibration with the spot moved
        /*! The Heston parameters and the leverage function are held, as for
            a delta of the calibrated model.
        */
        FxSLVPricingContext withSpot(Real spot) const;

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

        //! \name Inspectors
        //@{
        const FxPairMarket& market() const { return market_; }
        //! The calibrated Heston model, per year of trading time.
        ext::shared_ptr<HestonModel> hestonModel() const;
        //! The calibrated leverage function L(t, S), t in trading time.
        ext::shared_ptr<LocalVolTermStructure> leverageFunction() const;
        //! Per-pillar implied-vol calibration errors from the Heston fit.
        std::vector<Real> hestonCalibrationErrors() const;
        //! RMSE of the Heston fit across all calibration pillars.
        Real hestonRmse() const;
        //! Mixing factor of the leverage calibration, which the engines use too.
        Real mixingFactor() const { return mixingFactor_; }
        //! The Dupire local vol the leverage is calibrated against, in trading time.
        const Handle<LocalVolTermStructure>& localVol() const { return localVol_; }
        //@}

      private:
        FxPairMarket market_;
        Handle<YieldTermStructure> domesticTS_, foreignTS_;
        Handle<LocalVolTermStructure> localVol_;

        ext::shared_ptr<HestonModel>          hestonModel_;
        ext::shared_ptr<LocalVolTermStructure> leverageFunction_;
        std::vector<Real>                      hestonErrors_;
        Real                                   hestonRmse_ = 0.0;
        Real                                   mixingFactor_ = 1.0;
    };

}

#endif
