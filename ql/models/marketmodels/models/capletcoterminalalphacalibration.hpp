/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#ifndef quantlib_caplet_coterminal_alpha_calibration_hpp
#define quantlib_caplet_coterminal_alpha_calibration_hpp

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class PiecewiseConstantVariance;
    class Matrix;
    class AlphaForm;

    bool calibrationOfAlphaFunctionData(
        const EvolutionDescription& evolution,
        const PiecewiseConstantCorrelation& corr,
        const std::vector<boost::shared_ptr<
            PiecewiseConstantVariance> >&
                displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const CurveState& cs,
        const Spread displacement,
        const Size numberOfFactors,
        boost::shared_ptr<AlphaForm> parametricform,
        const std::vector<Real>& alphaInitial,
        const std::vector<Real>& alphaMax,
        const std::vector<Real>& alphaMin,
        Integer steps,
        Real toleranceForAlphaSolving,
        bool maximizeHomogeneity,
        std::vector<Real>& alpha, // for info only,
        std::vector<Real>& a,     // for info only,
        std::vector<Real>& b,    // for info only,
        std::vector<Matrix>& swapCovariancePseudoRoots // the real result
     );

    bool calibrationOfAlphaFunctionDataIterative(
        const EvolutionDescription& evolution,
        const PiecewiseConstantCorrelation& corr,
        const std::vector<boost::shared_ptr<
        PiecewiseConstantVariance> >&
        displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const CurveState& cs,
        const Spread displacement,
        const Size numberOfFactors,
        boost::shared_ptr<AlphaForm> parametricform,
        const std::vector<Real>& alphaInitial,
        const std::vector<Real>& alphaMax,
        const std::vector<Real>& alphaMin,
        Integer steps,
        Real toleranceForAlphaSolving,
        bool maximizeHomogeneity,
        Size maxIterations,
        Real toleranceForIterativeSolving,
        std::vector<Real>& alpha, // for info only,
        std::vector<Real>& a,     // for info only,
        std::vector<Real>& b,     // for info only,
        std::vector<Matrix>& swapCovariancePseudoRoots // the real result
        );

    // doesn't return as much for those not interested in all the extras
    bool calibrationOfAlphaFunction(
        const EvolutionDescription& evolution,
        const PiecewiseConstantCorrelation& corr,
        const std::vector<boost::shared_ptr<
        PiecewiseConstantVariance> >&
        displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const CurveState& cs,
        const Spread displacement,
        const Size numberOfFactors,
        boost::shared_ptr<AlphaForm> parametricform,
        const std::vector<Real>& alphaInitial,
        const std::vector<Real>& alphaMax,
        const std::vector<Real>& alphaMin,
        Integer steps,
        Real toleranceForAlphaSolving,
        Size maxIterations,
        Real toleranceForIterativeSolving,
        std::vector<Matrix>& swapCovariancePseudoRoots);

}

#endif
