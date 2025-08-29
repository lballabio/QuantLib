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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/models/marketmodels/models/capletcoterminalswaptioncalibration.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

    CTSMMCapletOriginalCalibration::CTSMMCapletOriginalCalibration(
                            const EvolutionDescription& evolution,
                            const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
                            const std::vector<ext::shared_ptr<
                                        PiecewiseConstantVariance> >&
                                                displacedSwapVariances,
                            const std::vector<Volatility>& mktCapletVols,
                            const ext::shared_ptr<CurveState>& cs,
                            Spread displacement,
                            const std::vector<Real>& alpha,
                            bool lowestRoot,
                            bool useFullApprox)
    : CTSMMCapletCalibration(evolution, corr, displacedSwapVariances,
                             mktCapletVols, cs, displacement),
      alpha_(alpha), lowestRoot_(lowestRoot),
      useFullApprox_(useFullApprox) {

        QL_REQUIRE(numberOfRates_==alpha.size(),
                   "mismatch between number of rates (" << numberOfRates_ <<
                   ") and alpha (" << alpha.size() << ")");

    }

    Natural CTSMMCapletOriginalCalibration::calibrationFunction(
                            const EvolutionDescription& evolution,
                            const PiecewiseConstantCorrelation& corr,
                            const std::vector<ext::shared_ptr<
                                PiecewiseConstantVariance> >&
                                    displacedSwapVariances,
                            const std::vector<Volatility>& capletVols,
                            const CurveState& cs,
                            Spread displacement,

                            const std::vector<Real>& alpha,
                            bool lowestRoot,
                            bool useFullAprox,

                            Size numberOfFactors,
                            //Size maxIterations,
                            //Real tolerance,

                            std::vector<Matrix>& swapCovariancePseudoRoots) {

        CTSMMCapletCalibration::performChecks(evolution, corr,
            displacedSwapVariances, capletVols, cs);

        Size numberOfSteps = evolution.numberOfSteps();
        Size numberOfRates = evolution.numberOfRates();
        const std::vector<Time>& rateTimes = evolution.rateTimes();

        QL_REQUIRE(numberOfFactors<=numberOfRates,
                   "number of factors (" << numberOfFactors <<
                   ") cannot be greater than numberOfRates (" <<
                   numberOfRates << ")");
        QL_REQUIRE(numberOfFactors>0,
                   "number of factors (" << numberOfFactors <<
                   ") must be greater than zero");

        Natural failures = 0;
        Real extraMultiplier = useFullAprox ? 1.0 : 0.0;

        // factor reduction
        std::vector<Matrix> corrPseudo(corr.times().size());
        for (Size i=0; i<corrPseudo.size(); ++i)
            corrPseudo[i] = rankReducedSqrt(corr.correlation(i),
                                            numberOfFactors, 1.0,
                                            SalvagingAlgorithm::None);

        Matrix zedMatrix =
            SwapForwardMappings::coterminalSwapZedMatrix(cs, displacement);
        Matrix invertedZedMatrix = inverse(zedMatrix);



        // do alpha part
        // first modify variances to take account of alpha
        // then rescale so total variance is unchanged
        Matrix swapTimeInhomogeneousVariances(numberOfSteps, numberOfRates, 0.0);
        std::vector<Real> originalVariances(numberOfRates, 0.0);
        std::vector<Real> modifiedVariances(numberOfRates, 0.0);
        const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
        for (Size i=0; i<numberOfSteps; ++i) {
            Real s = (i==0 ? 0.0 : evolutionTimes[i-1]);
            for (Size j=i; j<numberOfRates; ++j) {
                const std::vector<Real>& var = displacedSwapVariances[j]->variances();
                originalVariances[j]+=var[i];
                swapTimeInhomogeneousVariances[i][j] = var[i]/
                    ((1.0+alpha[j]*s)*(1.0+alpha[j]*s));
                modifiedVariances[j]+=swapTimeInhomogeneousVariances[i][j];
            }
        }

        for (Size i=0; i<numberOfSteps; ++i)
            for (Size j=i; j<numberOfRates; ++j)
                swapTimeInhomogeneousVariances[i][j] *= originalVariances[j]/
                                                        modifiedVariances[j];


        // compute swap covariances for caplet approximation formula
        // without taking into account A and B
        std::vector<Matrix> CovarianceSwapPseudos(numberOfSteps);
        std::vector<Matrix> CovarianceSwapCovs(numberOfSteps); // this is total cov
        std::vector<Matrix> CovarianceSwapMarginalCovs(numberOfSteps); // this is cov for one step

        for (Size i=0; i<numberOfSteps; ++i) {
            CovarianceSwapPseudos[i] =  corrPseudo[i];
            for (Size j=0; j<numberOfRates; ++j)
                for (Size k=0; k < CovarianceSwapPseudos[i].columns();  ++k)
                    CovarianceSwapPseudos[i][j][k] *=
                            std::sqrt(swapTimeInhomogeneousVariances[i][j]);

            CovarianceSwapMarginalCovs[i] = CovarianceSwapPseudos[i] *
                                    transpose(CovarianceSwapPseudos[i]);

            CovarianceSwapCovs[i] = CovarianceSwapMarginalCovs[i];
            if (i>0)
                CovarianceSwapCovs[i]+= CovarianceSwapCovs[i-1];
        }

        // compute partial variances and covariances which will take A and B coefficients
        //const std::vector<Time>& taus = evolution.rateTaus();
        std::vector<Real> totVariance(numberOfRates, 0.0);
        std::vector<Real> almostTotVariance(numberOfRates, 0.0);
        std::vector<Real> almostTotCovariance(numberOfRates, 0.0);
        std::vector<Real> leftCovariance(numberOfRates, 0.0);
        for (Size i=0; i<numberOfRates; ++i) {
            for (Size jj=0; jj<=i; ++jj)
                totVariance[i] += displacedSwapVariances[i]->variances()[jj];
            Integer j;
            for (j=0; j<=static_cast<Integer>(i)-1; ++j)
                almostTotVariance[i] += swapTimeInhomogeneousVariances[j][i];
            for (j=0; j<=static_cast<Integer>(i)-2; ++j) {
                const Matrix& thisPseudo = corrPseudo[j];
                Real correlation = 0.0;
                for (Size k=0; k<numberOfFactors; ++k)
                    correlation += thisPseudo[i-1][k]*thisPseudo[i][k];
                almostTotCovariance[i] += correlation *
                    std::sqrt(swapTimeInhomogeneousVariances[j][i] *
                    swapTimeInhomogeneousVariances[j][i-1]);
            }
            if (i>0) {
                const Matrix& thisPseudo = corrPseudo[j];
                Real correlation = 0.0;
                for (Size k=0; k<numberOfFactors; ++k)
                    correlation += thisPseudo[i-1][k]*thisPseudo[i][k];
                leftCovariance[i] = correlation *
                    std::sqrt(swapTimeInhomogeneousVariances[j][i] *
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

            // now compute contribution from top row excluding first two columns and its transpose
            // we divide into two parts as one part is multiplied by a[i-1] and the other by b[i-1]
            // a lot could be precomputed when we need to optimize
            for (Size k = i+1; k < numberOfSteps; ++k)
            {
                if (i > 1)
                {
                    extraConstantPart+=invertedZedMatrix[i-1][i-1] *
                        CovarianceSwapCovs[i-2][i-1][k] *
                        invertedZedMatrix[i-1][k]*a[i-1];

                    extraConstantPart+=invertedZedMatrix[i-1][k] *
                        CovarianceSwapCovs[i-2][k][i-1] *
                        invertedZedMatrix[i-1][i-1]*a[i-1];
                }

                extraConstantPart+=invertedZedMatrix[i-1][i-1] *
                    CovarianceSwapMarginalCovs[i-1][i-1][k] *
                    invertedZedMatrix[i-1][k]*b[i-1];

                extraConstantPart+=invertedZedMatrix[i-1][k] *
                    CovarianceSwapCovs[i-1][k][i-1] *
                    invertedZedMatrix[i-1][i-1]*b[i-1];

            }

            // we also get an extra linear part, this corresponds to row i, and columns j>i+1, and transpose
            Real extraLinearPart=0.0;
            for (Size k = i+1; k < numberOfSteps; ++k)
            {
                    extraLinearPart+=invertedZedMatrix[i-1][k] *
                        CovarianceSwapCovs[i-1][k][i] *
                        invertedZedMatrix[i-1][i];

                    extraLinearPart+=invertedZedMatrix[i-1][i] *
                        CovarianceSwapCovs[i-1][i][k] *
                        invertedZedMatrix[i-1][k];
            }



            Real constantPart = w0*w0*totVariance[i-1] +
                                                    extraConstantPart*extraMultiplier-v1t1;
            Real linearPart = -2*w0*w1*(a[i-1]*almostTotCovariance[i] +
                                                b[i-1]*leftCovariance[i]) +extraLinearPart*extraMultiplier ;
            Real quadraticPart = w1*w1*almostTotVariance[i];
            Real disc = linearPart*linearPart-4.0*constantPart*quadraticPart;

            Real root, minimum = -linearPart/(2.0*quadraticPart);
            bool rightUsed = false;
            if (disc <0.0) {
                ++failures;
                // pick up the minimum vol for the caplet
                root = minimum;
            } else if (lowestRoot) {
                root = (-linearPart-std::sqrt(disc))/(2.0*quadraticPart);
            } else {
                if (minimum>1.0)
                    root = (-linearPart-std::sqrt(disc))/(2.0*quadraticPart);
                else {
                    rightUsed = true;
                    root = (-linearPart+std::sqrt(disc))/(2.0*quadraticPart);
                }
            }

            Real varianceFound = root*root*almostTotVariance[i];
            Real varianceToFind = totVariance[i]-varianceFound;
            Real mult = varianceToFind/swapTimeInhomogeneousVariances[i][i];
            if (mult<=0.0 && rightUsed) {
                root = (-linearPart-std::sqrt(disc))/(2.0*quadraticPart);
                varianceFound = root*root*almostTotVariance[i];
                varianceToFind = totVariance[i]-varianceFound;
                mult = varianceToFind/swapTimeInhomogeneousVariances[i][i];
            }

            if (mult<0.0) // no solution...
            {
               ++failures;
               a[i]=root;
               b[i]=0.0;
            }
            else
            {
                a[i]=root;
                b[i]=std::sqrt(mult);
            }

            QL_ENSURE(root>=0.0,
                      "negative root -- it should have not happened");

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
            swapCovariancePseudoRoots[k] = corrPseudo[k];
            for (Size j=0; j<numberOfRates; ++j) {
                Real coeff = std::sqrt(swapTimeInhomogeneousVariances[k][j]);
                 for (Size i=0; i<numberOfFactors; ++i)
                    swapCovariancePseudoRoots[k][j][i]*=coeff;
            }
            QL_ENSURE(swapCovariancePseudoRoots[k].rows()==numberOfRates,
                      "step " << k
                      << " abcd vol wrong number of rows: "
                      << swapCovariancePseudoRoots[k].rows()
                      << " instead of " << numberOfRates);
            QL_ENSURE(swapCovariancePseudoRoots[k].columns()==numberOfFactors,
                      "step " << k
                      << " abcd vol wrong number of columns: "
                      << swapCovariancePseudoRoots[k].columns()
                      << " instead of " << numberOfFactors);
        }

        return failures;
    }

    Natural CTSMMCapletOriginalCalibration::calibrationImpl_(
                                Natural numberOfFactors,
                                Natural ,
                                Real ) {

        return calibrationFunction(evolution_,
                                   *corr_,
                                   displacedSwapVariances_,
                                   // not mktCapletVols_ but...
                                   usedCapletVols_,
                                   *cs_,
                                   displacement_,

                                   alpha_,
                                   lowestRoot_,
                                   useFullApprox_,

                                   numberOfFactors,

                                   swapCovariancePseudoRoots_);
    }

}
