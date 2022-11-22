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

#include <ql/models/marketmodels/models/capletcoterminalmaxhomogeneity.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/basisincompleteordered.hpp>
#include <ql/math/optimization/spherecylinder.hpp>
#include <ql/math/quadratic.hpp>

namespace QuantLib {

    namespace {

        bool singleRateClosestPointFinder(
            Size capletNumber,
            const std::vector<Volatility>& homogeneousSolution,
            const std::vector<Volatility>& previousRateSolution,
            Real capletVariance,
            const std::vector<Real>& correlations,
            Real w0,
            Real w1,
            Real capletSwaptionPriority,
            Size maxIterations,
            Real tolerance,
            std::vector<Volatility>& solution,
            Real finalWeight,
            Real& swaptionError,
            Real& capletError)
        {
            if (capletNumber ==0) // there only is one point so to go through everything would be silly
            {
                Real previousSwapVariance = previousRateSolution[0] *previousRateSolution[0];
                Real thisSwapVariance = homogeneousSolution[0] *homogeneousSolution[0]
                + homogeneousSolution[1] *homogeneousSolution[1];
                Real crossTerm =  2*w0*w1*correlations[0]*previousRateSolution[0];
                Real constantTerm = w0*w0* previousSwapVariance - capletVariance;
                Real theta = w1*w1;

                quadratic q(theta,crossTerm, constantTerm);
                Real volminus, volplus;
                bool capSuccess = q.roots(volminus,volplus);
                Real residual = thisSwapVariance - volminus*volminus;
                bool swapSuccess = residual >=0;
                bool success = capSuccess && swapSuccess;

                if (success)
                {
                    solution[0]  = volminus;
                    solution[1]  = std::sqrt(residual);
                    swaptionError= 0.0;
                    capletError =0.0;
                    return success;
                }

                bool prioritizeCaplet = capletSwaptionPriority <0.5;

                if (capSuccess && prioritizeCaplet)
                {
                    solution[0]  =  volminus;
                    solution[1]  =  0; // residual is negative or we'd have totally succeeded
                    swaptionError =  std::sqrt(thisSwapVariance)-volminus;
                    capletError =0.0;
                    return success;
                }

                if (capSuccess && !prioritizeCaplet)
                {
                    solution[0] = std::sqrt(thisSwapVariance);
                    solution[1] = 0.0;
                    swaptionError=0.0;
                    capletError= std::sqrt(q(solution[0])+capletVariance) - std::sqrt(capletVariance);
                    return success;
                }


                // ok caplets have failed
                if (swapSuccess)
                {
                    solution[0]  = volminus;
                    solution[1]  = std::sqrt(residual);
                    swaptionError= 0.0;
                    capletError= std::sqrt(q(solution[0])+capletVariance) - std::sqrt(capletVariance);
                    return success;
                }

                // ok caplets have failed and swaps fail with optimal caplet solution

                if (prioritizeCaplet)
                {
                    solution[0]  = volminus; 
                    solution[1]  =  0; // residual is negative or we'd have totally succeeded
                    swaptionError =  std::sqrt(thisSwapVariance)-volminus;
                    capletError =0.0;
  
                }
                else
                {
                    solution[0] = std::sqrt(thisSwapVariance);
                    solution[1] = 0.0;
                    swaptionError=0.0;
                    capletError= std::sqrt(q(solution[0])+capletVariance) - std::sqrt(capletVariance);


                }


                return success;
            }

            // first get in correct format
            Real previousSwapVariance=0.0;
            Real thisSwapVariance =0.0;
            {
                Size i=0;
                for (; i<capletNumber+1; ++i) {
                    previousSwapVariance += previousRateSolution[i] *
                        previousRateSolution[i];
                    thisSwapVariance += homogeneousSolution[i] *
                        homogeneousSolution[i];
                }
                thisSwapVariance+= homogeneousSolution[i]*homogeneousSolution[i];
            }

            Real theta = w1*w1;
            std::vector<Real> b(capletNumber+1);
            Array cylinderCentre(capletNumber+1);
            Array targetArray(capletNumber+2);
            Array targetArrayRestricted(capletNumber+1);


            Real bsq = 0.0;
            for (Size i=0; i<capletNumber+1; ++i) {
                b[i] = 2*w0*w1*correlations[i]*previousRateSolution[i]/theta;
                cylinderCentre[i] = -0.5*b[i];
                targetArray[i] = homogeneousSolution[i];
                targetArrayRestricted[i] = targetArray[i];
                bsq+=b[i]*b[i];
            }
            targetArray[capletNumber+1] = homogeneousSolution[capletNumber+1];

            Real A = previousSwapVariance*w0*w0/theta;
            Real constQuadraticTerm = A - 0.25*bsq;
            Real S2 = capletVariance/theta - constQuadraticTerm;

            // if S2 < 0, there are no solutions so we take the best we can. 
            Real S = S2 > 0 ? Real(std::sqrt(S2)) : 0;

            Real R = std::sqrt(thisSwapVariance);

            BasisIncompleteOrdered basis(capletNumber+1);
            basis.addVector(cylinderCentre);
            basis.addVector(targetArrayRestricted);
            for (Size i=0; i<capletNumber+1; ++i) {
                Array ei(capletNumber+1,0.0);
                ei[i]=1.0;
                basis.addVector(ei);
            }

            Matrix orthTransformationRestricted(basis.getBasisAsRowsInMatrix());
            Matrix orthTransformation(capletNumber+2, capletNumber+2, 0.0);

            orthTransformation[capletNumber+1][capletNumber+1]=1.0;
            for (Size k=0; k<capletNumber+1; ++k)
                for (Size l=0; l<capletNumber+1; ++l)
                    orthTransformation[k][l]=orthTransformationRestricted[k][l];

            Array movedCentre = orthTransformationRestricted*cylinderCentre;
            Real alpha = movedCentre[0];
            Array movedTarget = orthTransformation*targetArray;

            Real Z1=0.0, Z2=0.0, Z3=0.0;

            SphereCylinderOptimizer optimizer(R, S, alpha, movedTarget[0], movedTarget[1],movedTarget[movedTarget.size()-1],finalWeight);

            bool success = false;

            if (!optimizer.isIntersectionNonEmpty())
            {
                Z1 = R*capletSwaptionPriority+(1-capletSwaptionPriority)*(alpha-S);
                Z2 = 0.0;
                Z3 = 0.0;
                swaptionError =Z1-R;
                capletError = (alpha-S)-Z1;
            }
            else
            {
                success = true;
                capletError =0.0;
                swaptionError =0.0;

                if (maxIterations > 0.0)
                {
                    optimizer.findClosest(maxIterations,
                        tolerance,
                        Z1,
                        Z2,
                        Z3);
                }
                else
                    optimizer.findByProjection(
                    Z1,
                    Z2,
                    Z3);
            }

            Array rotatedSolution(capletNumber+2,0.0);
            rotatedSolution[0] = Z1;
            rotatedSolution[1] = Z2;
            rotatedSolution[capletNumber+1] = Z3;

            Array arraySolution(transpose(orthTransformation) *
                rotatedSolution);
            {
                Size i=0;
                for (; i < arraySolution.size(); ++i)
                    solution[i]=arraySolution[i];
                for (; i < solution.size(); ++i)
                    solution[i]=0.0;
            }

            return success;

        }

    }

    CTSMMCapletMaxHomogeneityCalibration::CTSMMCapletMaxHomogeneityCalibration(
        const EvolutionDescription& evolution,
        const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
        const std::vector<ext::shared_ptr<
        PiecewiseConstantVariance> >&
        displacedSwapVariances,
        const std::vector<Volatility>& mktCapletVols,
        const ext::shared_ptr<CurveState>& cs,
        Spread displacement,
        Real caplet0Swaption1Priority)
        : CTSMMCapletCalibration(evolution, corr, displacedSwapVariances,
        mktCapletVols, cs, displacement),
        caplet0Swaption1Priority_(caplet0Swaption1Priority) {

            QL_REQUIRE(caplet0Swaption1Priority>=0.0 &&
                caplet0Swaption1Priority<=1.0,
                "caplet0Swaption1Priority (" << caplet0Swaption1Priority <<
                ") must be in [0.0, 1.0]");
    }

    // the actual calibration function, this is a static class member
    Natural CTSMMCapletMaxHomogeneityCalibration::capletMaxHomogeneityCalibration(
        const EvolutionDescription& evolution,
        const PiecewiseConstantCorrelation& corr,
        const std::vector<ext::shared_ptr<
        PiecewiseConstantVariance> >& displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const CurveState& cs,
        const Spread displacement,
        Real caplet0Swaption1Priority, 
        const Size numberOfFactors,
        Size maxIterations,
        Real tolerance,
        Real& deformationSize,  // ret value
        Real& totalSwaptionError, // ret value
        std::vector<Matrix>& swapCovariancePseudoRoots) 
    {

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


            Natural failures=0;

            totalSwaptionError = 0.0;
            deformationSize = 0.0;

            // factor reduction
            std::vector<Matrix> corrPseudo(corr.times().size());
            for (Size i=0; i<corrPseudo.size(); ++i)
                corrPseudo[i] = rankReducedSqrt(corr.correlation(i),
                numberOfFactors, 1.0,
                SalvagingAlgorithm::None);

            // get Zinverse, we can get wj later
            Matrix zedMatrix =
                SwapForwardMappings::coterminalSwapZedMatrix(cs, displacement);
            Matrix invertedZedMatrix = inverse(zedMatrix);

            // vectors for the new vol of all swap rates
            std::vector<std::vector<Volatility> > newVols;
            std::vector<Volatility> theseNewVols(numberOfRates);
            std::vector<Volatility> firstRateVols(numberOfRates);
            firstRateVols[0] = std::sqrt(displacedSwapVariances[0]->variances()[0]);
            std::vector<Volatility> secondRateVols(numberOfRates);
            std::vector<Real> correlations(numberOfRates);
            newVols.push_back(firstRateVols);

            // final caplet and swaption are the same, so we skip that case
            for (Size i=0; i<numberOfRates-1; ++i) 
            {
                // final weight dont do anything when i < 2 
                Real thisFinalWeight = i  > 1 ? (i-1)/2.0 : 1.0;
                // we will calibrate caplet on forward rate i,
                // we will do this by modifying the vol of swap rate i+1
                const std::vector<Real>& var =
                    displacedSwapVariances[i+1]->variances();

                for (Size j =0; j < i+2; ++j)
                    secondRateVols[j] = std::sqrt(var[j]);

                for (Size k=0; k < i+1; k++) {
                    Real correlation=0.0;
                    for (Size l=0; l < numberOfFactors; ++l) {
                        Real term1 = corrPseudo[k][i][l];
                        Real term2 = corrPseudo[k][i+1][l];
                        correlation += term1*term2;
                    }
                    correlations[k] = correlation;
                }

                Real w0 = invertedZedMatrix[i][i];
                Real w1 = invertedZedMatrix[i][i+1];
                // w0 adjustment
                for (Size k = i+2; k <invertedZedMatrix.columns(); ++k)
                    w0+= invertedZedMatrix[i][k];

                Real targetCapletVariance= capletVols[i]*capletVols[i]*rateTimes[i];

                Real thisCapletError;
                Real thisSwaptionError;

                bool success = singleRateClosestPointFinder(
                    i, secondRateVols, firstRateVols, targetCapletVariance, correlations,
                    w0, w1, caplet0Swaption1Priority,maxIterations, tolerance,
                    theseNewVols, thisFinalWeight ,thisSwaptionError, thisCapletError);

                totalSwaptionError+= thisSwaptionError*thisSwaptionError;

                if (!success)
                    ++failures;

                for (Size j=0; j < i+2; ++j)
                    deformationSize += (theseNewVols[i]-secondRateVols[i])*(theseNewVols[i]-secondRateVols[i]);

                newVols.push_back(theseNewVols);
                firstRateVols = theseNewVols;
            }

            swapCovariancePseudoRoots.resize(numberOfSteps);
            for (Size k=0; k<numberOfSteps; ++k) {
                swapCovariancePseudoRoots[k] = corrPseudo[k];
                for (Size j=0; j<numberOfRates; ++j) {
                    Real coeff =newVols[j][k];
                    for (Size i=0; i<numberOfFactors; ++i)
                        swapCovariancePseudoRoots[k][j][i]*=coeff;
                }
                QL_ENSURE(swapCovariancePseudoRoots[k].rows()==numberOfRates,
                    "step " << k << " abcd vol wrong number of rows: " <<
                    swapCovariancePseudoRoots[k].rows() <<
                    " instead of " << numberOfRates);
                QL_ENSURE(swapCovariancePseudoRoots[k].columns()==numberOfFactors,
                    "step " << k << " abcd vol wrong number of columns: " <<
                    swapCovariancePseudoRoots[k].columns() <<
                    " instead of " << numberOfFactors);
            }

            return failures;
    }

    Natural CTSMMCapletMaxHomogeneityCalibration::calibrationImpl_(
        Natural numberOfFactors, 
        Natural maxIterations,
        Real tolerance) {

            return capletMaxHomogeneityCalibration(evolution_,
                *corr_,
                displacedSwapVariances_,
                // not mktCapletVols_ but...
                usedCapletVols_,
                *cs_, 
                displacement_, 

                caplet0Swaption1Priority_,

                numberOfFactors,
                maxIterations,
                tolerance,

                deformationSize_,
                totalSwaptionError_,

                swapCovariancePseudoRoots_);
    }

}
