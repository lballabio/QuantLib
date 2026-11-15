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

#include <ql/experimental/fxslv/slvleveragecalibrator.hpp>
#include <ql/methods/finitedifferences/operators/fdmsquarerootfwdop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmhestongreensfct.hpp>

namespace QuantLib {

    HestonSLVLeverageCalibrator::HestonSLVLeverageCalibrator(
        HestonSLVFokkerPlanckFdmParams params,
        Real mixingFactor,
        bool logging)
    : params_(params), mixingFactor_(mixingFactor), logging_(logging) {
        // the mixing factor scales the vol-of-vol; at 0 the variance is
        // deterministic, which HestonSLVFDMModel's variance grid cannot handle
        QL_REQUIRE(mixingFactor_ > 0.0 && mixingFactor_ <= 1.0,
                   "HestonSLVLeverageCalibrator: mixingFactor must be in (0,1], got "
                   << mixingFactor_);
    }

    HestonSLVFokkerPlanckFdmParams HestonSLVLeverageCalibrator::defaultParams() {
        // Balanced grid suitable for FX option books with maturities up to ~5 years.
        // - xGrid 201: log-spot dimension; adequate for 5+ sigma range
        // - vGrid 201: variance dimension; the leverage function, through
        //   E[v | S], needs it far finer than the spot one
        // - tMaxStepsPerYear 365: daily steps at short end for accuracy
        // - tMinStepsPerYear 25: coarser at long end for efficiency
        // - tStepNumberDecay 3.0: smooth ramp from daily to monthly steps
        // - nRannacherTimeSteps 0: no implicit Euler steps at the start.  They
        //   would damp a Dirac initial density, but the density starts as a
        //   Gaussian Green's function, already smooth; and their iterative
        //   solver (BiCGStab) fails to converge on fine grids, e.g. 201 x 201
        //   with rho -0.6, where without them the calibration converges, to
        //   the same leverage, several times faster
        // - predictionCorrectionSteps 2: standard predictor-corrector
        // - x0Density 0.1: probability mass assigned to the initial log-spot cell
        // - localVolEpsProb 1e-4: density floor below which local vol is not evaluated
        // - maxIntegrationIterations 10000: Gaussian quadrature limit
        // - vLowerEps/vUpperEps: variance mesher boundary tolerance
        // - vMin: absolute variance floor
        // - v0Density 1.0: initial variance density concentration
        // - vLowerBoundDensity / vUpperBoundDensity: mesher boundary densities
        // - leverageFctPropEps 1e-6: density threshold below which L = 1 (avoids 0/0)
        // - Power transformation of the variance density: with the Log one,
        //   SLV calibrated on an 11-pillar EURUSD surface misses the vanillas
        //   by 0.3% at the money and 1% at 105% (vol of vol 0.15, trading
        //   time); with Power, which also suits densities piling up at zero
        //   variance when the Feller condition fails, by under 0.1%
        return {
            201, 201,           // xGrid, vGrid
            365, 25,            // tMaxStepsPerYear, tMinStepsPerYear
            3.0,                // tStepNumberDecay
            0,                  // nRannacherTimeSteps
            2,                  // predictionCorrectionSteps
            0.1,                // x0Density
            1.0e-4,             // localVolEpsProb
            10000,              // maxIntegrationIterations
            1.0e-8,             // vLowerEps
            1.0e-8,             // vUpperEps
            0.0,                // vMin
            1.0,                // v0Density
            0.1,                // vLowerBoundDensity
            0.9,                // vUpperBoundDensity
            1.0e-6,             // leverageFctPropEps
            FdmHestonGreensFct::Gaussian,
            FdmSquareRootFwdOp::Power,
            FdmSchemeDesc::ModifiedCraigSneyd()
        };
    }

    void HestonSLVLeverageCalibrator::setStochVolModel(
        const ext::shared_ptr<CalibratedModel>& model) {
        auto heston = ext::dynamic_pointer_cast<HestonModel>(model);
        QL_REQUIRE(heston,
                   "HestonSLVLeverageCalibrator::setStochVolModel: "
                   "model must be a HestonModel");
        hestonModel_ = heston;
    }

    void HestonSLVLeverageCalibrator::setHestonModel(
        const ext::shared_ptr<HestonModel>& model) {
        QL_REQUIRE(model, "HestonSLVLeverageCalibrator::setHestonModel: null model");
        hestonModel_ = model;
    }

    void HestonSLVLeverageCalibrator::calibrate(
        const Handle<LocalVolTermStructure>& localVol,
        const Date& endDate,
        const std::vector<Date>& mandatoryDates) {

        QL_REQUIRE(hestonModel_,
                   "HestonSLVLeverageCalibrator: Heston model has not been set; "
                   "call setStochVolModel() or setHestonModel() before calibrate()");
        QL_REQUIRE(!localVol.empty(),
                   "HestonSLVLeverageCalibrator: local vol handle is empty");

        fdmModel_ = ext::make_shared<HestonSLVFDMModel>(
            localVol,
            Handle<HestonModel>(hestonModel_),
            endDate,
            params_,
            logging_,
            mandatoryDates,
            mixingFactor_);
    }

    ext::shared_ptr<LocalVolTermStructure>
    HestonSLVLeverageCalibrator::leverageFunction() const {
        QL_REQUIRE(fdmModel_,
                   "HestonSLVLeverageCalibrator: calibrate() has not been called");
        return fdmModel_->leverageFunction();
    }

    ext::shared_ptr<HestonSLVFDMModel> HestonSLVLeverageCalibrator::fdmModel() const {
        QL_REQUIRE(fdmModel_,
                   "HestonSLVLeverageCalibrator: calibrate() has not been called");
        return fdmModel_;
    }

}
