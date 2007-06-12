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

#ifndef quantlib_caplet_coterminal_max_homogeneity_calibration_hpp
#define quantlib_caplet_coterminal_max_homogeneity_calibration_hpp

#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class PiecewiseConstantVariance;
    class Matrix;


    class CapletCoterminalSwaptionCalibration3 {
      public:
        CapletCoterminalSwaptionCalibration3(
            const EvolutionDescription& evolution,
            const boost::shared_ptr<PiecewiseConstantCorrelation>& corr,
            const std::vector<boost::shared_ptr<
                        PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const boost::shared_ptr<CurveState>& cs,
            Spread displacement);
        // modifiers
        bool calibrate(Size numberOfFactors,

                       Size maxIterations,
                       Real capletVolTolerance,

                       Real caplet0Swaption1Priority,
                       Size iterationsForHomogeneous = 100,
                       Real toleranceHomogeneousSolving = 1e-8);
        // inspectors
        Real deformationSize() const;
        Real rmsCapletError() const; 
        Real rmsSwaptionError() const;
        const std::vector<Matrix>& swapPseudoRoots() const;
        const Matrix& swapPseudoRoot(Size i) const;
        // actual calibration function
        static Size calibrationOfMaxHomogeneity(
                    const EvolutionDescription& evolution,
                    const PiecewiseConstantCorrelation& corr,
                    const std::vector<boost::shared_ptr<
                        PiecewiseConstantVariance> >&
                            displacedSwapVariances,
                    const std::vector<Volatility>& capletVols,
                    const CurveState& cs,
                    const Spread displacement,
                    Size numberOfFactors,
                    Real capletSwaptionPriority, 
                 

                    Size iterationsForHomogeneous,
                    Real toleranceHomogeneousSolving,

                    Real& deformationSize,
                    std::vector<Matrix>& swapCovariancePseudoRoots,
                    Real& totalSwaptionError);
      private:
        // input
        EvolutionDescription evolution_;
        boost::shared_ptr<PiecewiseConstantCorrelation> corr_;
        std::vector<boost::shared_ptr<PiecewiseConstantVariance> >
                                                displacedSwapVariances_;
        std::vector<Volatility> mktCapletVols_;
        boost::shared_ptr<CurveState> cs_;
        Spread displacement_;
        // results
        bool calibrated_;
        Real error_, swaptionError_, deformationSize_;
        Size failures_;
        std::vector<Matrix> swapCovariancePseudoRoots_;
    };

}

#endif
