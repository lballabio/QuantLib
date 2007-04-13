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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_caplet_coterminal_calibration_hpp
#define quantlib_caplet_coterminal_calibration_hpp

#include <ql/marketmodels/curvestate.hpp>
#include <ql/marketmodels/evolutiondescription.hpp>
#include <ql/marketmodels/models/timedependantcorrelationstructure.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class PiecewiseConstantVariance;
    class Matrix;

    bool capletCoterminalCalibration(
                            const EvolutionDescription& evolution,
                            const TimeDependantCorrelationStructure& corr,
                            const std::vector<boost::shared_ptr<
                                PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
                            const std::vector<Volatility>& capletVols,
                            const CurveState& cs,
                            const Spread displacement,
                            const std::vector<Real>& alpha,
                            const bool lowestRoot,
                            std::vector<Matrix>& swapCovariancePseudoRoots,
                            Size& negativeDiscriminants);

    bool iterativeCapletCoterminalCalibration(
            const EvolutionDescription& evolution,
            const TimeDependantCorrelationStructure& corr,
            const std::vector<boost::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
            const std::vector<Volatility>& displacedCapletVols,
            const CurveState& cs,
            const Spread displacement,
            const std::vector<Real>& alpha,
            const bool lowestRoot,
            std::vector<Matrix>& swapCovariancePseudoRoots,
            const Size maxIterations,
            const Real tolerance);


    class IterativeCapletCoterminalCalibration {
      public:
        IterativeCapletCoterminalCalibration(
            const EvolutionDescription& evolution,
            const boost::shared_ptr<TimeDependantCorrelationStructure>& corr,
            const std::vector<boost::shared_ptr<
                        PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const boost::shared_ptr<CurveState>& cs,
            Spread displacement);
        // modifiers
        bool calibrate(const std::vector<Real>& alpha,
                       bool lowestRoot,
                       Size maxIterations,
                       Real tolerance);
        // inspectors
        Size negativeDiscriminants();
        Real error();
        const std::vector<Matrix>& swapCovariancePseudoRoots();
        const Matrix& swapCovariancePseudoRoot(Size i);
      private:
        // input
        EvolutionDescription evolution_;
        boost::shared_ptr<TimeDependantCorrelationStructure> corr_;
        std::vector<boost::shared_ptr<PiecewiseConstantVariance> >
                                                displacedSwapVariances_;
        std::vector<Volatility> mktCapletVols_;
        boost::shared_ptr<CurveState> cs_;
        Spread displacement_;
        // results
        bool calibrated_;
        Size negDiscr_;
        Real error_;
        std::vector<Matrix> swapCovariancePseudoRoots_;
    };

}

#endif
