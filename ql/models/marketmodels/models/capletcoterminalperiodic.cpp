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

#include <ql/models/marketmodels/models/capletcoterminalperiodic.hpp>
#include <ql/models/marketmodels/models/capletcoterminalmaxhomogeneity.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/volatilityinterpolationspecifier.hpp>
#include <ql/math/matrix.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <ql/models/marketmodels/models/pseudorootfacade.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/models/fwdperiodadapter.hpp>
#include <ql/models/marketmodels/models/fwdtocotswapadapter.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib
{

    // for the displaced swap

    Integer capletSwaptionPeriodicCalibration(
        const EvolutionDescription& evolution,
        const boost::shared_ptr<PiecewiseConstantCorrelation>& corr,
        VolatilityInterpolationSpecifier&
        displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const boost::shared_ptr<CurveState>& cs,
        const Spread displacement,
        Real caplet0Swaption1Priority,
        Size numberOfFactors,
        Size period,
        Size max1dIterations,
        Real tolerance1d,
        Size maxUnperiodicIterations,
        Real toleranceUnperiodic,
        Size maxPeriodIterations,
        Real periodTolerance,
        Real& , // deformationSize used to return information, not set yet
        Real& totalSwaptionError, // ?
        std::vector<Matrix>& swapCovariancePseudoRoots,  // the thing we really want the pseudo root for each time step
        std::vector<Real> & finalScales,  //scalings used for matching
        Size& iterationsDone, // number of  period iteratations done
        Real& errorImprovement, // improvement in error for last iteration
        Matrix& modelSwaptionVolsMatrix // the swaption vols calibrated to at each step of the iteration
        )
    {

        Size numberSmallRates = evolution.numberOfRates();
        Size numberSmallSteps = evolution.numberOfSteps();

        QL_REQUIRE( numberSmallSteps == numberSmallRates,
            "periodic calibration class requires evolution to the reset of each rate");

        Size numberBigRates = numberSmallRates/period;
        Size offset = numberSmallRates % period;

        std::vector<Spread> newDisplacements(numberBigRates,displacement);

        QL_REQUIRE(displacedSwapVariances.getNoBigRates() == numberBigRates,
            "mismatch between number of swap variances given and number of rates and period");

        Integer failures=0;


        std::vector<Real> scalingFactors(numberBigRates);
        for (Size i=0; i < numberBigRates; ++i)
            scalingFactors[i] =1.0;

        displacedSwapVariances.setLastCapletVol(*capletVols.rbegin());


        std::vector<Real> marketSwaptionVols(numberBigRates);
        for (Size i=0; i < numberBigRates; ++i) {
            marketSwaptionVols[i] =
                displacedSwapVariances.originalVariances()[i]->totalVolatility(i);
        }

        std::vector<Real> modelSwaptionVols(numberBigRates);

        Real periodSwaptionRmsError;

        iterationsDone = 0;

        Real previousError = 1.0e+10; // very large number


        modelSwaptionVolsMatrix =Matrix(maxPeriodIterations,numberBigRates,0.0);


        do
        {
            displacedSwapVariances.setScalingFactors(scalingFactors);


            CTSMMCapletMaxHomogeneityCalibration unperiodicCalibrator(
                evolution,
                corr,
                displacedSwapVariances.interpolatedVariances(),
                capletVols,
                cs,
                displacement,
                caplet0Swaption1Priority);


            failures= unperiodicCalibrator.calibrate(numberOfFactors, maxUnperiodicIterations,toleranceUnperiodic,max1dIterations,tolerance1d);

            swapCovariancePseudoRoots = unperiodicCalibrator.swapPseudoRoots();

            boost::shared_ptr<MarketModel> smm(new
                PseudoRootFacade(swapCovariancePseudoRoots,
                evolution.rateTimes(),
                cs->coterminalSwapRates(),
                std::vector<Spread>(evolution.numberOfRates(), displacement)));

            boost::shared_ptr<MarketModel> flmm(new CotSwapToFwdAdapter(smm));

            Matrix capletTotCovariance = flmm->totalCovariance(numberSmallRates-1);

            boost::shared_ptr<MarketModel> periodflmm( new FwdPeriodAdapter(flmm, period,
                offset,
                newDisplacements));

            boost::shared_ptr<MarketModel> periodsmm(new FwdToCotSwapAdapter(periodflmm));


            Matrix swaptionTotCovariance(periodsmm->totalCovariance(periodsmm->numberOfSteps()-1));



            totalSwaptionError=0.0;

            for (Size i=0; i < numberBigRates; ++i)
            {
                modelSwaptionVols[i] = std::sqrt(swaptionTotCovariance[i][i]/periodsmm->evolution().rateTimes()[i]);
                Real scale = marketSwaptionVols[i]/modelSwaptionVols[i];
                scalingFactors[i] *= scale; // since applied to vol

                totalSwaptionError +=  (marketSwaptionVols[i]-modelSwaptionVols[i])* (marketSwaptionVols[i]-modelSwaptionVols[i]);

            }

            for (Size i=0; i < numberBigRates; ++i)
                modelSwaptionVolsMatrix[iterationsDone][i] = modelSwaptionVols[i];

            periodSwaptionRmsError = std::sqrt(totalSwaptionError/numberBigRates);
            errorImprovement = previousError -periodSwaptionRmsError;
            previousError = periodSwaptionRmsError;
        }
        while (errorImprovement> periodTolerance/10.0 && periodSwaptionRmsError >periodTolerance && ++iterationsDone < maxPeriodIterations);

        finalScales = scalingFactors;

        return failures;

    }


}
