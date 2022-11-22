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

#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/models/marketmodels/models/alphafinder.hpp>
#include <ql/models/marketmodels/models/alphaformconcrete.hpp>
#include <ql/models/marketmodels/models/capletcoterminalalphacalibration.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <utility>

namespace QuantLib {

    CTSMMCapletAlphaFormCalibration::CTSMMCapletAlphaFormCalibration(
        const EvolutionDescription& evolution,
        const ext::shared_ptr<PiecewiseConstantCorrelation>& corr,
        const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
        const std::vector<Volatility>& mktCapletVols,
        const ext::shared_ptr<CurveState>& cs,
        Spread displacement,
        const std::vector<Real>& alphaInitial,
        const std::vector<Real>& alphaMax,
        const std::vector<Real>& alphaMin,
        bool maximizeHomogeneity,
        ext::shared_ptr<AlphaForm> parametricForm)
    : CTSMMCapletCalibration(
          evolution, corr, displacedSwapVariances, mktCapletVols, cs, displacement),
      alphaInitial_(alphaInitial), alphaMax_(alphaMax), alphaMin_(alphaMin),
      maximizeHomogeneity_(maximizeHomogeneity), parametricForm_(std::move(parametricForm)),
      alpha_(numberOfRates_), a_(numberOfRates_), b_(numberOfRates_) {
        if (!parametricForm_)
            parametricForm_ =
                ext::shared_ptr<AlphaForm>(new AlphaFormLinearHyperbolic(evolution.rateTimes()));

        QL_REQUIRE(numberOfRates_==alphaInitial.size(),
            "mismatch between number of rates (" << numberOfRates_ <<
            ") and alphaInitial (" << alphaInitial.size() << ")");

        QL_REQUIRE(numberOfRates_==alphaMax.size(),
            "mismatch between number of rates (" << numberOfRates_ <<
            ") and alphaMax (" << alphaMax.size() << ")");

        QL_REQUIRE(numberOfRates_==alphaMin.size(),
            "mismatch between number of rates (" << numberOfRates_ <<
            ") and alphaMin (" << alphaMin.size() << ")");
    }

    Natural CTSMMCapletAlphaFormCalibration::capletAlphaFormCalibration(
        const EvolutionDescription& evolution,
        const PiecewiseConstantCorrelation& corr,
        const std::vector<ext::shared_ptr<PiecewiseConstantVariance> >& displacedSwapVariances,
        const std::vector<Volatility>& capletVols,
        const CurveState& cs,
        const Spread displacement,

        const std::vector<Real>& alphaInitial,
        const std::vector<Real>& alphaMax,
        const std::vector<Real>& alphaMin,
        bool maximizeHomogeneity,
        const ext::shared_ptr<AlphaForm>& parametricForm,

        const Size numberOfFactors,
        Integer maxIterations,
        Real tolerance,

        std::vector<Real>& alpha,
        std::vector<Real>& a,
        std::vector<Real>& b,

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

        Natural failures=0;

        alpha.resize(numberOfRates);
        a.resize(numberOfRates);
        b.resize(numberOfRates);

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

        // vectors for new vol
        std::vector<std::vector<Volatility> > newVols;
        std::vector<Volatility> theseNewVols(numberOfRates);
        std::vector<Volatility> firstRateVols(numberOfRates);
        firstRateVols[0] = std::sqrt(displacedSwapVariances[0]->variances()[0]);
        std::vector<Volatility> secondRateVols(numberOfRates);
        std::vector<Real> correlations(numberOfRates);
        newVols.push_back(firstRateVols);


        alpha[0] = alphaInitial[0]; // has no effect on anything in any case
        a[0] = b[0] = 1.0; // no modifications to swap vol for first rate

        AlphaFinder solver(parametricForm);

        // final caplet and swaption are the same, so we skip that case
        for (Size i=0; i<numberOfRates-1; ++i) {
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

            Real targetVariance= capletVols[i]*capletVols[i]*rateTimes[i];

            bool success;
            if (maximizeHomogeneity)
                success = solver.solveWithMaxHomogeneity(
                                                    alphaInitial[i+1] ,
                                                    i,
                                                    firstRateVols,
                                                    secondRateVols,
                                                    correlations,
                                                    w0,
                                                    w1,
                                                    targetVariance,
                                                    tolerance,
                                                    alphaMax[i+1],
                                                    alphaMin[i+1],
                                                    maxIterations,
                                                    alpha[i+1],
                                                    a[i+1],
                                                    b[i+1],
                                                    theseNewVols);
            else
                success = solver.solve(alphaInitial[i+1] ,
                                       i,
                                       firstRateVols,
                                       secondRateVols,
                                       correlations,
                                       w0,
                                       w1,
                                       targetVariance,
                                       tolerance,
                                       alphaMax[i+1],
                                       alphaMin[i+1],
                                       maxIterations,
                                       alpha[i+1],
                                       a[i+1],
                                       b[i+1],
                                       theseNewVols);

            if (!success) {
                //++failures;
                QL_FAIL("alpha form failure");
            }

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

    Natural CTSMMCapletAlphaFormCalibration::calibrationImpl_(
                                Natural numberOfFactors,
                                Natural maxIterations,
                                Real tolerance) {

        return capletAlphaFormCalibration(evolution_,
                                          *corr_,
                                          displacedSwapVariances_,
                                          // not mktCapletVols_ but...
                                          usedCapletVols_,
                                          *cs_,
                                          displacement_,

                                          alphaInitial_,
                                          alphaMax_,
                                          alphaMin_,
                                          maximizeHomogeneity_,
                                          parametricForm_,

                                          numberOfFactors,
                                          maxIterations,
                                          tolerance,

                                          alpha_,
                                          a_,
                                          b_,

                                          swapCovariancePseudoRoots_);
    }
}
