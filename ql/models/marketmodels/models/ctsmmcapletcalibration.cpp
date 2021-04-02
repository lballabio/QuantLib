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

#include <ql/math/comparison.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/models/ctsmmcapletcalibration.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/pseudorootfacade.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    CTSMMCapletCalibration::CTSMMCapletCalibration(
        EvolutionDescription evolution,
        ext::shared_ptr<PiecewiseConstantCorrelation> corr,
        std::vector<ext::shared_ptr<PiecewiseConstantVariance> > displacedSwapVariances,
        std::vector<Volatility> mktCapletVols,
        ext::shared_ptr<CurveState> cs,
        Spread displacement)
    : evolution_(std::move(evolution)), corr_(std::move(corr)),
      displacedSwapVariances_(std::move(displacedSwapVariances)),
      mktCapletVols_(std::move(mktCapletVols)), mdlCapletVols_(evolution_.numberOfRates()),
      mktSwaptionVols_(evolution_.numberOfRates()), mdlSwaptionVols_(evolution_.numberOfRates()),
      cs_(std::move(cs)), displacement_(displacement), numberOfRates_(evolution_.numberOfRates()) {
        performChecks(evolution_, *corr_, displacedSwapVariances_,
                      mktCapletVols_, *cs_);
    }

    const std::vector<Volatility>&
    CTSMMCapletCalibration::timeDependentUnCalibratedSwaptionVols(Size i) const
    {
        QL_REQUIRE(i<numberOfRates_,
                   "index (" << i << ") must less than number of rates (" <<
                   numberOfRates_ << ")");
        return displacedSwapVariances_[i]->volatilities();
    }

    const std::vector<Volatility>&
    CTSMMCapletCalibration::timeDependentCalibratedSwaptionVols(Size i) const
    {
        QL_REQUIRE(i<numberOfRates_,
                   "index (" << i << ") must less than number of rates (" <<
                   numberOfRates_ << ")");
        return timeDependentCalibratedSwaptionVols_[i];
    }

    void CTSMMCapletCalibration::performChecks(
                    const EvolutionDescription& evolution,
                    const PiecewiseConstantCorrelation&  corr,
                    const std::vector<ext::shared_ptr<
                                PiecewiseConstantVariance> >&
                                            displacedSwapVariances,
                    const std::vector<Volatility>& mktCapletVols,
                    const CurveState& cs)
    {
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        QL_REQUIRE(evolutionTimes==corr.times(),
                   "evolutionTimes "
                   << io::sequence(evolutionTimes)
                   << " not equal to correlation times "
                   << io::sequence(corr.times()));

        const std::vector<Time>& rateTimes = evolution.rateTimes();
        QL_REQUIRE(rateTimes==cs.rateTimes(),
                   "mismatch between EvolutionDescription and "
                   "CurveState rate times");

        Size numberOfRates = evolution.numberOfRates();
        QL_REQUIRE(numberOfRates==displacedSwapVariances.size(),
                   "mismatch between EvolutionDescription number of rates ("
                   << numberOfRates << ") and displacedSwapVariances size ("
                   << displacedSwapVariances.size() << ")");
        QL_REQUIRE(numberOfRates==corr.numberOfRates(),
                   "mismatch between EvolutionDescription number of rates ("
                   << numberOfRates << ") and corr number of rates ("
                   << corr.numberOfRates() << ")");
        QL_REQUIRE(numberOfRates==mktCapletVols.size(),
                   "mismatch between EvolutionDescription number of rates ("
                   << numberOfRates << ") and mktCapletVols size ("
                   << mktCapletVols.size() << ")");
        QL_REQUIRE(numberOfRates==cs.numberOfRates(),
                   "mismatch between EvolutionDescription number of rates ("
                   << numberOfRates << ") and CurveState  number of rates ("
                   << cs.numberOfRates() << ")");

        std::vector<Time> temp(rateTimes.begin(), rateTimes.end()-1);
        QL_REQUIRE(temp==evolutionTimes,
                   "mismatch between evolutionTimes and rateTimes");

        Volatility lastSwaptionVol =
            displacedSwapVariances.back()->totalVolatility(numberOfRates-1);
        QL_REQUIRE(close(lastSwaptionVol, mktCapletVols[numberOfRates-1]),
                   "last caplet vol (" << std::setprecision(16) <<
                   mktCapletVols[numberOfRates-1] <<
                   ") must be equal to last swaption vol (" <<
                   lastSwaptionVol << "); discrepancy is " <<
                   lastSwaptionVol-mktCapletVols[numberOfRates-1]);
    }

    bool CTSMMCapletCalibration::calibrate(Natural numberOfFactors,

                                           Natural maxIterations,
                                           Real capletVolTolerance,

                                           Natural innerSolvingMaxIterations,
                                           Real innerSolvingTolerance) {
        // initialize results
        calibrated_ = false;
        failures_ = 987654321; // a positive large number
        deformationSize_ = 987654321;
        capletRmsError_ = swaptionRmsError_ = 987654321;
        capletMaxError_ = swaptionMaxError_ = 987654321;

        // initialize working variables
        usedCapletVols_ = mktCapletVols_;

        for (Size i=0; i<numberOfRates_; ++i)
            mktSwaptionVols_[i]=displacedSwapVariances_[i]->totalVolatility(i);

        std::vector<Spread> displacements(numberOfRates_,
                                          displacement_);
        const std::vector<Time>& rateTimes = evolution_.rateTimes();
        Natural iterations = 0;

        // calibration loop
        do {
            failures_ = calibrationImpl_(numberOfFactors,
                                         innerSolvingMaxIterations,
                                         innerSolvingTolerance);

            ext::shared_ptr<MarketModel> ctsmm(new
                PseudoRootFacade(swapCovariancePseudoRoots_,
                                 rateTimes,
                                 cs_->coterminalSwapRates(),
                                 displacements));
            const Matrix& swaptionTotCovariance =
                ctsmm->totalCovariance(numberOfRates_-1);

            CotSwapToFwdAdapter flmm(ctsmm);
            const Matrix& capletTotCovariance =
                flmm.totalCovariance(numberOfRates_-1);

            // check fit
            capletRmsError_ = swaptionRmsError_ = 0.0;
            capletMaxError_ = swaptionMaxError_ = -1.0;

            for (Size i=0; i<numberOfRates_; ++i) {
                mdlSwaptionVols_[i] = std::sqrt(swaptionTotCovariance[i][i]/rateTimes[i]);
                Real swaptionError = std::fabs(mktSwaptionVols_[i]-mdlSwaptionVols_[i]);
                swaptionRmsError_ += swaptionError*swaptionError;
                swaptionMaxError_ = std::max(swaptionMaxError_, swaptionError);

                mdlCapletVols_[i] = std::sqrt(capletTotCovariance[i][i]/rateTimes[i]);
                Real capletError = std::fabs(mktCapletVols_[i]-mdlCapletVols_[i]);
                capletRmsError_ += capletError*capletError;
                capletMaxError_ = std::max(capletMaxError_, capletError);

                if (i < numberOfRates_-1)
                    usedCapletVols_[i] *= mktCapletVols_[i]/mdlCapletVols_[i];
            }
            swaptionRmsError_ = std::sqrt(swaptionRmsError_/numberOfRates_);
            capletRmsError_ = std::sqrt(capletRmsError_/numberOfRates_);
            ++iterations;
        } while (iterations<maxIterations &&
                 capletRmsError_>capletVolTolerance);

         ext::shared_ptr<MarketModel> ctsmm(new
                PseudoRootFacade(swapCovariancePseudoRoots_,
                                 rateTimes,
                                 cs_->coterminalSwapRates(),
                                 displacements));

         timeDependentCalibratedSwaptionVols_.clear();
         for (Size i=0; i<numberOfRates_; ++i)
             timeDependentCalibratedSwaptionVols_.push_back(
                ctsmm->timeDependentVolatility(i));

        // calculate deformationSize_ ??
        calibrated_ = true;
        return failures_==0;
    }

}
