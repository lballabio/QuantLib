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

/*! \file hestoncalibrator.hpp
    \brief Heston stochastic volatility model calibrator
*/

#ifndef quantlib_heston_calibrator_hpp
#define quantlib_heston_calibrator_hpp

#include <ql/experimental/fxslv/stochvolcalibrator.hpp>
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/models/calibrationhelper.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Heston model initial parameter guess.
    struct HestonParams {
        Real v0    = 0.04;   //!< initial variance (≈ ATM vol²)
        Real kappa = 1.0;    //!< mean reversion speed
        Real theta = 0.04;   //!< long-run variance
        Real sigma = 0.5;    //!< volatility of variance (vol-of-vol)
        Real rho   = -0.5;   //!< spot-variance correlation
    };

    //! Calibrates a Heston stochastic volatility model to vanilla option pillars.
    /*!
        Builds one \c HestonModelHelper per supplied pillar, attaches an
        \c AnalyticHestonEngine for pricing, and minimises the weighted
        sum of squared implied-vol errors using Levenberg-Marquardt.

        The Feller condition \f$ 2\kappa\theta > \sigma^2 \f$, which keeps
        the variance away from zero, is not enforced by default: FX smiles
        usually need more vol-of-vol than it allows, and the SLV
        finite-difference model handles variances reaching zero.  It can be
        enforced on request.

        The optimiser cannot leave a starting point that violates the
        constraints, as every trial around it is penalised alike, so such
        a start is rejected, as is a fit that ends without moving or on the
        iteration limit.

        After a successful call to \c calibrate(), the results are available
        via \c calibratedHestonModel(), \c calibrationErrors(), and
        \c impliedVolSurface().

        \ingroup experimental
    */
    class HestonCalibrator : public StochVolCalibrator {
      public:
        //! \name Construction
        //@{
        /*!
            \param initialParams    starting guess for Heston parameters
            \param endCriteria      optimisation stopping criteria
            \param errorType        cost function metric (implied vol recommended)
            \param fixParameters    mask for parameters to hold fixed during
                                    optimisation (length 5: theta, kappa, sigma,
                                    rho, v0; \c false = free, \c true = fixed)
            \param fellerConstraint whether to enforce the Feller condition
        */
        explicit HestonCalibrator(
            HestonParams initialParams = HestonParams{},
            EndCriteria endCriteria =
                EndCriteria(1000, 500, 1.0e-8, 1.0e-8, 1.0e-8),
            BlackCalibrationHelper::CalibrationErrorType errorType =
                BlackCalibrationHelper::ImpliedVolError,
            std::vector<bool> fixParameters = {},
            bool fellerConstraint = false);
        //@}

        //! \name StochVolCalibrator interface
        //@{
        void calibrate(const Handle<Quote>&                 spot,
                       const Handle<YieldTermStructure>&    domesticTS,
                       const Handle<YieldTermStructure>&    foreignTS,
                       const Handle<BlackVolTermStructure>& volSurface,
                       const Calendar&                      calendar,
                       const std::vector<Pillar>&           pillars) override;

        ext::shared_ptr<CalibratedModel> calibratedModel() const override;
        std::vector<Real> calibrationErrors() const override;
        ext::shared_ptr<BlackVolTermStructure> impliedVolSurface() const override;
        //@}

        //! Convenience accessor returning the model as its concrete type.
        ext::shared_ptr<HestonModel> calibratedHestonModel() const;

        //! The calibration helpers constructed during \c calibrate().
        const std::vector<ext::shared_ptr<BlackCalibrationHelper>>& helpers() const;

      private:
        const HestonParams initialParams_;
        const EndCriteria endCriteria_;
        const BlackCalibrationHelper::CalibrationErrorType errorType_;
        const std::vector<bool> fixParameters_;
        const bool fellerConstraint_;

        ext::shared_ptr<HestonModel> hestonModel_;
        std::vector<ext::shared_ptr<BlackCalibrationHelper>> helpers_;
        std::vector<Real> calibrationErrors_;
        ext::shared_ptr<BlackVolTermStructure> impliedVolSurface_;
    };

}

#endif
