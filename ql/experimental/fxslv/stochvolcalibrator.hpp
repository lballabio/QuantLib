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

/*! \file stochvolcalibrator.hpp
    \brief Abstract interface for stochastic volatility model calibrators
*/

#ifndef quantlib_stoch_vol_calibrator_hpp
#define quantlib_stoch_vol_calibrator_hpp

#include <ql/handle.hpp>
#include <ql/models/model.hpp>
#include <ql/quote.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Abstract base class for stochastic volatility model calibrators.
    /*!
        Provides a uniform interface for calibrating stochastic volatility
        models (e.g. Heston, Bates, rough volatility) to a set of vanilla
        option market instruments.  Concrete subclasses supply the model-
        specific calibration mechanics while the rest of the SLV pipeline
        (leverage function calibration, pricing context) can be written
        against this interface.

        Calibration instruments are supplied as a grid of (expiry, strike)
        pairs together with the corresponding market implied volatilities,
        which are queried from the provided Black vol surface.

        \ingroup experimental
    */
    class StochVolCalibrator {
      public:
        virtual ~StochVolCalibrator() = default;

        //! Calibration instrument specification: one vanilla option pillar.
        struct Pillar {
            Period expiry;  //!< option tenor, e.g. 3*Months
            Real   strike;  //!< absolute strike in domestic currency
        };

        //! Run the calibration.
        /*!
            \param spot         handle to the current FX spot quote
            \param domesticTS   domestic (pricing) yield term structure
            \param foreignTS    foreign (dividend / foreign risk-free) yield term structure
            \param volSurface   market Black implied volatility surface
            \param calendar     calendar for expiry date generation
            \param pillars       (expiry, strike) pairs defining calibration instruments
        */
        virtual void calibrate(const Handle<Quote>&                        spot,
                               const Handle<YieldTermStructure>&           domesticTS,
                               const Handle<YieldTermStructure>&           foreignTS,
                               const Handle<BlackVolTermStructure>&        volSurface,
                               const Calendar&                             calendar,
                               const std::vector<Pillar>&                  pillars) = 0;

        //! Returns the calibrated model.
        /*! Throws if calibrate() has not yet been called. */
        virtual ext::shared_ptr<CalibratedModel> calibratedModel() const = 0;

        //! Per-pillar implied-vol errors (model vol minus market vol).
        /*! Empty until calibrate() is called.  NaN for a pillar whose model
            price has no implied vol, so that a failure cannot pass for a
            perfect fit.
        */
        virtual std::vector<Real> calibrationErrors() const = 0;

        //! Root-mean-square implied vol error across all pillars; NaN if any pillar's is.
        Real rmse() const;

        //! Black volatility surface implied by the calibrated model.
        /*!
            Useful for plotting model vs market smile. Throws if calibrate()
            has not yet been called.
        */
        virtual ext::shared_ptr<BlackVolTermStructure> impliedVolSurface() const = 0;
    };

}

#endif
