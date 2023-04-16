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

#ifndef quantlib_ctsmm_caplet_calibration_max_homogeneity_hpp
#define quantlib_ctsmm_caplet_calibration_max_homogeneity_hpp

#include <ql/models/marketmodels/models/ctsmmcapletcalibration.hpp>

namespace QuantLib {

    class CTSMMCapletMaxHomogeneityCalibration : public CTSMMCapletCalibration {
      public:
        CTSMMCapletMaxHomogeneityCalibration(
            const EvolutionDescription& evolution,
            const std::shared_ptr<PiecewiseConstantCorrelation>& corr,
            const std::vector<std::shared_ptr<
                        PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const std::shared_ptr<CurveState>& cs,
            Spread displacement,
            Real caplet0Swaption1Priority = 1.0);

        // actual calibration function
        static Natural capletMaxHomogeneityCalibration(
            const EvolutionDescription& evolution,
            const PiecewiseConstantCorrelation& corr,
            const std::vector<std::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const CurveState& cs,
            Spread displacement,
            Real caplet0Swaption1Priority,
            Size numberOfFactors,
            Size maxIterations,
            Real tolerance,
            Real& deformationSize,
            Real& totalSwaptionError,                        // ?
            std::vector<Matrix>& swapCovariancePseudoRoots); // the thing we really want the pseudo
                                                             // root for each time step

      private:
        Natural
        calibrationImpl_(Natural numberOfFactors, Natural maxIterations, Real tolerance) override;
        // input
        Real caplet0Swaption1Priority_;
        // results
        Real totalSwaptionError_; // ??
    };

}

#endif
