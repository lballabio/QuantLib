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


#ifndef quantlib_ctsmm_caplet_calibration_periodic_hpp
#define quantlib_ctsmm_caplet_calibration_periodic_hpp


#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/volatilityinterpolationspecifier.hpp>
#include <ql/math/matrix.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {


    Integer capletSwaptionPeriodicCalibration(
        const EvolutionDescription& evolution,
        const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
        VolatilityInterpolationSpecifier& displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const ext::shared_ptr<CurveState>& cs,
        Spread displacement,
        Real caplet0Swaption1Priority,
        Size numberOfFactors,
        Size period,
        Size max1dIterations,
        Real tolerance1d,
        Size maxUnperiodicIterations,
        Real toleranceUnperiodic,
        Size maxPeriodIterations,
        Real periodTolerance,
        Real& deformationSize,                          // used to return information, not set yet
        Real& totalSwaptionError,                       // ?
        std::vector<Matrix>& swapCovariancePseudoRoots, // the thing we really want the pseudo root
                                                        // for each time step
        std::vector<Real>& finalScales,                 // scalings used for matching
        Size& iterationsDone,                           // number of  period iteratations done
        Real& errorImprovement,                         // improvement in error for last iteration
        Matrix&
            modelSwaptionVolsMatrix // the swaption vols calibrated to at each step of the iteration
    );
}

#endif
