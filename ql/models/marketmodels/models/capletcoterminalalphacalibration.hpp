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

    class CapletCoterminalSwaptionCalibration2 {
      public:
        CapletCoterminalSwaptionCalibration2(
            const EvolutionDescription& evolution,
            const boost::shared_ptr<PiecewiseConstantCorrelation>& corr,
            const std::vector<boost::shared_ptr<
                        PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const boost::shared_ptr<CurveState>& cs,
            Spread displacement,
            boost::shared_ptr<AlphaForm> parametricform =
                                    boost::shared_ptr<AlphaForm>());
        // modifiers
        bool calibrate(Size numberOfFactors,
                       Size maxIterations,
                       Real capletVolTolerance,
                       Size iterationsForAlphaSolving,
                       Real toleranceForAlphaSolving,
                       bool maximizeHomogeneity,
                       const std::vector<Real>& alphaInitial,
                       const std::vector<Real>& alphaMax,
                       const std::vector<Real>& alphaMin);
        // inspectors
        Real deformationSize() const;
        Real rmsError() const; // caplet
        const std::vector<Matrix>& swapPseudoRoots() const;
        const Matrix& swapPseudoRoot(Size i) const;
        // actual calibration function
        static bool calibrationOfAlphaFunctionData(
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
            std::vector<Real>& alpha,
            std::vector<Real>& a,
            std::vector<Real>& b,
            std::vector<Matrix>& swapCovariancePseudoRoots);
      private:
        // input
        EvolutionDescription evolution_;
        boost::shared_ptr<PiecewiseConstantCorrelation> corr_;
        std::vector<boost::shared_ptr<PiecewiseConstantVariance> >
                                                displacedSwapVariances_;
        std::vector<Volatility> mktCapletVols_;
        boost::shared_ptr<CurveState> cs_;
        Spread displacement_;
        boost::shared_ptr<AlphaForm> parametricform_;
        // results
        bool calibrated_;
        Real error_, deformationSize_;
        std::vector<Real> alpha_, a_, b_;
        std::vector<Matrix> swapCovariancePseudoRoots_;
    };

}

#endif
