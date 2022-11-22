/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_ctsmm_caplet_calibration_alfa_form_hpp
#define quantlib_ctsmm_caplet_calibration_alfa_form_hpp

#include <ql/models/marketmodels/models/ctsmmcapletcalibration.hpp>

namespace QuantLib {

    class AlphaForm;

    class CTSMMCapletAlphaFormCalibration : public CTSMMCapletCalibration {
      public:
        CTSMMCapletAlphaFormCalibration(
            const EvolutionDescription& evolution,
            const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
            const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const ext::shared_ptr<CurveState>& cs,
            Spread displacement,
            const std::vector<Real>& alphaInitial,
            const std::vector<Real>& alphaMax,
            const std::vector<Real>& alphaMin,
            bool maximizeHomogeneity,
            ext::shared_ptr<AlphaForm> parametricForm = ext::shared_ptr<AlphaForm>());
        // inspector
        const std::vector<Real>& alpha() const;
        // actual calibration function
        static Natural capletAlphaFormCalibration(
            const EvolutionDescription& evolution,
            const PiecewiseConstantCorrelation& corr,
            const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const CurveState& cs,
            Spread displacement,

            const std::vector<Real>& alphaInitial,
            const std::vector<Real>& alphaMax,
            const std::vector<Real>& alphaMin,
            bool maximizeHomogeneity,
            const ext::shared_ptr<AlphaForm>& parametricForm,

            Size numberOfFactors,
            Integer steps,
            Real toleranceForAlphaSolving,

            std::vector<Real>& alpha,
            std::vector<Real>& a,
            std::vector<Real>& b,

            std::vector<Matrix>& swapCovariancePseudoRoots);

      private:
        Natural
        calibrationImpl_(Natural numberOfFactors, Natural maxIterations, Real tolerance) override;
        // input
        std::vector<Real> alphaInitial_, alphaMax_, alphaMin_;
        bool maximizeHomogeneity_;
        ext::shared_ptr<AlphaForm> parametricForm_;
        // results
        std::vector<Real> alpha_, a_, b_;
    };

    inline const std::vector<Real>&
    CTSMMCapletAlphaFormCalibration::alpha() const {
        QL_REQUIRE(calibrated_, "not successfully calibrated yet");
        return alpha_;
    }

}

#endif
