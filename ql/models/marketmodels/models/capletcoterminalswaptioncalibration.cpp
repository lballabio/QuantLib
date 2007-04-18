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

#include <ql/models/marketmodels/models/capletcoterminalswaptioncalibration.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/pseudorootfacade.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

    CapletCoterminalSwaptionCalibration::CapletCoterminalSwaptionCalibration(
                            const EvolutionDescription& evolution,
                            const boost::shared_ptr<TimeDependantCorrelationStructure>& corr,
                            const std::vector<boost::shared_ptr<
                                        PiecewiseConstantVariance> >&
                                                displacedSwapVariances,
                            const std::vector<Volatility>& mktCapletVols,
                            const boost::shared_ptr<CurveState>& cs,
                            Spread displacement)
    : evolution_(evolution),  corr_(corr),
      displacedSwapVariances_(displacedSwapVariances),
      mktCapletVols_(mktCapletVols),
      cs_(cs), displacement_(displacement), calibrated_(false) {}

    bool CapletCoterminalSwaptionCalibration::calibrationFunction(
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
                            Size& negativeDiscriminants) {

        negativeDiscriminants = 0;
        QL_REQUIRE(evolution.evolutionTimes()==corr.times(),
                   "evolutionTimes not equal to correlation times");

        Size numberOfRates_ = evolution.numberOfRates();

        QL_REQUIRE(numberOfRates_==displacedSwapVariances.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and displacedSwapVariances");

        QL_REQUIRE(numberOfRates_==capletVols.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and capletVols (" << capletVols.size() <<
                   ")");

        const std::vector<Time>& rateTimes = evolution.rateTimes();
        QL_REQUIRE(rateTimes==cs.rateTimes(),
                   "mismatch between EvolutionDescriptionand CurveState rate times ");
        QL_REQUIRE(numberOfRates_==cs.numberOfRates(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and CurveState");

        QL_REQUIRE(numberOfRates_==alpha.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and alphas (" << alpha.size() << ")");

        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        QL_REQUIRE(std::vector<Time>(rateTimes.begin(), rateTimes.end()-1)==evolutionTimes,
                   "mismatch between evolutionTimes and rateTimes");

        Size numberOfSteps = evolution.numberOfSteps();
        Size numberOfFactors = corr.numberOfFactors();

        // do alpha part
        // first modify variances to take account of alpha
        // then rescale so total variance is unchanged
        Matrix swapTimeInhomogeneousVariances(numberOfSteps, numberOfRates_, 0.0);
        std::vector<Real> originalVariances(numberOfRates_, 0.0);
        std::vector<Real> modifiedVariances(numberOfRates_, 0.0);
        for (Size i=0; i<numberOfSteps; ++i) {
            Real s = (i==0 ? 0.0 : evolutionTimes[i-1]);
            for (Size j=i; j<numberOfRates_; ++j) {
                const std::vector<Real>& var = displacedSwapVariances[j]->variances();
                originalVariances[j]+=var[i];
                swapTimeInhomogeneousVariances[i][j] = var[i]/
                    ((1.0+alpha[j]*s)*(1.0+alpha[j]*s));
                modifiedVariances[j]+=swapTimeInhomogeneousVariances[i][j];
            }
        }

        for (Size i=0; i<numberOfSteps; ++i)
            for (Size j=i; j<numberOfRates_; ++j)
                swapTimeInhomogeneousVariances[i][j] *= originalVariances[j]/
                                                        modifiedVariances[j];

        Matrix zedMatrix =
            SwapForwardMappings::coterminalSwapZedMatrix(cs, displacement);
        Matrix invertedZedMatrix = inverse(zedMatrix);


        // compute swap covariances for caplet approximation formula
        // without taking into account A and B
        std::vector<Matrix> CovarianceSwapPseudos(numberOfSteps);
        std::vector<Matrix> CovarianceSwapCovs(numberOfSteps);

        for (Size i=0; i<numberOfSteps; ++i) {
            CovarianceSwapPseudos[i] =  corr.pseudoRoot(i);
            for (Size j=0; j<numberOfRates_; ++j)
                for (Size k=0; k < CovarianceSwapPseudos[i].columns();  ++k)
                    CovarianceSwapPseudos[i][j][k] *=
                            sqrt(swapTimeInhomogeneousVariances[i][j]);

            CovarianceSwapCovs[i] = CovarianceSwapPseudos[i] *
                                    transpose(CovarianceSwapPseudos[i]);
            if (i>0)
                CovarianceSwapCovs[i]+= CovarianceSwapCovs[i-1];
        }

        // compute partial variances and covariances which will take A and B coefficients
        const std::vector<Time>& taus = evolution.rateTaus();
        std::vector<Real> totVariance(numberOfRates_, 0.0);
        std::vector<Real> almostTotVariance(numberOfRates_, 0.0);
        std::vector<Real> almostTotCovariance(numberOfRates_, 0.0);
        std::vector<Real> leftCovariance(numberOfRates_, 0.0);
        for (Size i=0; i<numberOfRates_; ++i) {
            for (Size j=0; j<=i; ++j)
                totVariance[i] += displacedSwapVariances[i]->variances()[j];
            for (Integer j=0; j<=static_cast<Integer>(i)-1; ++j)
                almostTotVariance[i] += swapTimeInhomogeneousVariances[j][i];
            Integer j=0;
            for (; j<=static_cast<Integer>(i)-2; ++j) {
                const Matrix& thisPseudo = corr.pseudoRoot(j);
                Real correlation = 0.0;
                for (Size k=0; k<numberOfFactors; ++k)
                    correlation += thisPseudo[i-1][k]*thisPseudo[i][k];
                almostTotCovariance[i] += correlation *
                    sqrt(swapTimeInhomogeneousVariances[j][i] *
                    swapTimeInhomogeneousVariances[j][i-1]);
            }
            if (i>0) {
                const Matrix& thisPseudo = corr.pseudoRoot(j);
                Real correlation = 0.0;
                for (Size k=0; k<numberOfFactors; ++k)
                    correlation += thisPseudo[i-1][k]*thisPseudo[i][k];
                leftCovariance[i] = correlation *
                    sqrt(swapTimeInhomogeneousVariances[j][i] *
                    swapTimeInhomogeneousVariances[j][i-1]);
            }
        }


        // multiplier up to rate reset previous time
        // the first element is not used
        std::vector<Real> a(numberOfSteps, 1.0);
        // multiplier afterward
        std::vector<Real> b(numberOfSteps);

        b[0]=displacedSwapVariances[0]->variances()[0]/swapTimeInhomogeneousVariances[0][0];
        // main loop where work is done
        for (Size i=1; i<numberOfSteps; ++i) {
            Integer j=0;
            // up date variances to take account of last a and b computed
            for (; j <= static_cast<Integer>(i)-2; j++)
                swapTimeInhomogeneousVariances[j][i-1]*= a[i-1]*a[i-1];
            swapTimeInhomogeneousVariances[j][i-1]*= b[i-1]*b[i-1];

            Real w0 = invertedZedMatrix[i-1][i-1];
            Real w1 = -invertedZedMatrix[i-1][i];
            Real v1t1 = capletVols[i-1]*capletVols[i-1]*rateTimes[i-1];

            // now compute contribution from lower right corner
            Real extraConstantPart =0.0;
            // part of caplet approximation formula coming from later rates
            for (Size k = i+1; k < numberOfSteps; ++k)
                for (Size l = i+1; l < numberOfSteps; ++l)
                    extraConstantPart+=invertedZedMatrix[i-1][k] *
                                         CovarianceSwapCovs[i-1][k][l] *
                                         invertedZedMatrix[i-1][l];

            Real constantPart = w0*w0*totVariance[i-1] +
                                                    extraConstantPart-v1t1;
            Real linearPart = -2*w0*w1*(a[i-1]*almostTotCovariance[i] +
                                                b[i-1]*leftCovariance[i]);
            Real quadraticPart = w1*w1*almostTotVariance[i];
            Real disc = linearPart*linearPart-4.0*constantPart*quadraticPart;

            Real root, minimum = -linearPart/(2.0*quadraticPart);
            bool rightUsed = false;
            if (disc <0.0) {
                ++negativeDiscriminants;
                // pick up the minimum vol for the caplet
                root = minimum;
            } else if (lowestRoot) {
                root = (-linearPart-sqrt(disc))/(2.0*quadraticPart);
            } else {
                if (minimum>1.0)
                    root = (-linearPart-sqrt(disc))/(2.0*quadraticPart);
                else {
                    rightUsed = true;
                    root = (-linearPart+sqrt(disc))/(2.0*quadraticPart);
                }
            }

            Real varianceFound = root*root*almostTotVariance[i];
            Real varianceToFind = totVariance[i]-varianceFound;
            Real mult = varianceToFind/swapTimeInhomogeneousVariances[i][i];
            if (mult<=0.0 && rightUsed) {
                root = (-linearPart-sqrt(disc))/(2.0*quadraticPart);
                varianceFound = root*root*almostTotVariance[i];
                varianceToFind = totVariance[i]-varianceFound;
                mult = varianceToFind/swapTimeInhomogeneousVariances[i][i];
            }

            if (mult<0.0) // no solution...
                return false;

            QL_ENSURE(root>=0.0,
                      "negative root -- it should have not happened");

            a[i]=root;
            b[i]=std::sqrt(mult);
        }

        {
            Integer i = numberOfSteps;
            Integer j=0;
            for (; j <= static_cast<Integer>(i)-2; j++)
                swapTimeInhomogeneousVariances[j][i-1]*= a[i-1]*a[i-1];
            swapTimeInhomogeneousVariances[j][i-1]*= b[i-1]*b[i-1];
        }

        // compute the results
        swapCovariancePseudoRoots.resize(numberOfSteps);
        for (Size k=0; k<numberOfSteps; ++k) {
            swapCovariancePseudoRoots[k] = corr.pseudoRoot(k);
            for (Size j=0; j<numberOfRates_; ++j) {
                Real coeff = std::sqrt(swapTimeInhomogeneousVariances[k][j]);
                 for (Size i=0; i<numberOfFactors; ++i)
                    swapCovariancePseudoRoots[k][j][i]*=coeff;
            }
            QL_ENSURE(swapCovariancePseudoRoots[k].rows()==numberOfRates_,
                      "step " << k
                      << " abcd vol wrong number of rows: "
                      << swapCovariancePseudoRoots[k].rows()
                      << " instead of " << numberOfRates_);
            QL_ENSURE(swapCovariancePseudoRoots[k].columns()==numberOfFactors,
                      "step " << k
                      << " abcd vol wrong number of columns: "
                      << swapCovariancePseudoRoots[k].columns()
                      << " instead of " << numberOfFactors);
        }

        return true;
    }

    bool CapletCoterminalSwaptionCalibration::calibrate(
                            const std::vector<Real>& alpha,
                            bool lowestRoot,
                            Size maxIterations,
                            Real tolerance) {

        const std::vector<Time>& rateTimes = evolution_.rateTimes();
        Size numberOfRates = evolution_.numberOfRates();
        negDiscr_ = 0;
        error_ = 987654321; // a positive large number
        bool success = true;
        std::vector<Volatility> targetCapletVols(mktCapletVols_);
        for (Size iterCounter=1;
             iterCounter<=maxIterations && success && negDiscr_==0 && error_>tolerance;
             ++iterCounter) {
            success = calibrationFunction(evolution_,
                                          *corr_,
                                          displacedSwapVariances_,
                                          targetCapletVols,
                                          *cs_,
                                          displacement_,
                                          alpha,
                                          lowestRoot,
                                          swapCovariancePseudoRoots_,
                                          negDiscr_);
            boost::shared_ptr<MarketModel> smm(new
                PseudoRootFacade(swapCovariancePseudoRoots_,
                                 rateTimes,
                                 cs_->coterminalSwapRates(),
                                 std::vector<Spread>(numberOfRates, displacement_)));
            CotSwapToFwdAdapter flmm(smm);
            // avoid this copy
            Matrix capletTotCovariance = flmm.totalCovariance(numberOfRates-1);
            error_ = 0.0;
            for (Size i=0; i<numberOfRates-1; ++i) {
                Real capletVol = std::sqrt(capletTotCovariance[i][i]/rateTimes[i]);
                Real diff = mktCapletVols_[i]-capletVol;
                error_ += diff*diff;
                targetCapletVols[i] *= mktCapletVols_[i]/capletVol;
            }
            error_ = std::sqrt(error_/(numberOfRates-1));
        }
        calibrated_ = true;
        return success;
    }

    Size CapletCoterminalSwaptionCalibration::negativeDiscriminants() const {
        QL_REQUIRE(calibrated_, "not calibrated yet");
        return negDiscr_;
    }

    Real CapletCoterminalSwaptionCalibration::rmsError() const {
        QL_REQUIRE(calibrated_, "not calibrated yet");
        return error_;
    }

    const std::vector<Matrix>&
    CapletCoterminalSwaptionCalibration::swapPseudoRoots() const {
        QL_REQUIRE(calibrated_, "not calibrated yet");
        return swapCovariancePseudoRoots_;
    }

    const Matrix&
    CapletCoterminalSwaptionCalibration::swapPseudoRoot(Size i) const {
        QL_REQUIRE(calibrated_, "not calibrated yet");
        QL_REQUIRE(i<swapCovariancePseudoRoots_.size(),
                   "invalid index");
        return swapCovariancePseudoRoots_[i];
    }

}
