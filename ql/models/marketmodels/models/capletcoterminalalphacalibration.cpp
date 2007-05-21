/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

#include <ql/models/marketmodels/models/capletcoterminalalphacalibration.hpp>
#include <ql/models/marketmodels/models/alphafinder.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/pseudorootfacade.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {

	bool calibrationOfAlphaFunctionData(
            const EvolutionDescription& evolution,
            const PiecewiseConstantCorrelation& corr,
            const std::vector<boost::shared_ptr<
            PiecewiseConstantVariance> >&
            displacedSwapVariances,
            const std::vector<Volatility>& capletVols,
            const CurveState& cs,
            const Spread displacement,
            const Size numberOfFactors,
            boost::shared_ptr<alphaform> parametricform,
            const std::vector<Real>& alphaInitial,
            const std::vector<Real>& alphaMax,
            const std::vector<Real>& alphaMin,
            Integer steps,
            Real toleranceForAlphaSolving,
            bool maximizeHomogeneity,
            std::vector<Real>& alpha,
            std::vector<Real>& a,
            std::vector<Real>& b,
            std::vector<Matrix>& swapCovariancePseudoRoots) {

        QL_REQUIRE(evolution.evolutionTimes()==corr.times(),
            "evolutionTimes not equal to correlation times");

    	Size numberOfRates = evolution.numberOfRates();
    	QL_REQUIRE(numberOfFactors<=numberOfRates,
            "number of factors (" << numberOfFactors <<
            ") cannot be greater than numberOfRates (" <<
        	numberOfRates << ")");
    	QL_REQUIRE(numberOfFactors>0,
            "number of factors (" << numberOfFactors <<
            ") must be greater than zero");

    	QL_REQUIRE(numberOfRates==displacedSwapVariances.size(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and displacedSwapVariances");

    	QL_REQUIRE(numberOfRates==capletVols.size(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and capletVols (" << capletVols.size() <<
            ")");

    	const std::vector<Time>& rateTimes = evolution.rateTimes();
    	QL_REQUIRE(rateTimes==cs.rateTimes(),
            "mismatch between EvolutionDescriptionand CurveState rate times ");
    	QL_REQUIRE(numberOfRates==cs.numberOfRates(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and CurveState");

    	QL_REQUIRE(numberOfRates==alphaInitial.size(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and alphas (" << alphaInitial.size() << ")");

    	QL_REQUIRE(numberOfRates==alphaMax.size(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and alphas (" << alphaMax.size() << ")");

    	QL_REQUIRE(numberOfRates==alphaMin.size(),
            "mismatch between number of rates (" << numberOfRates <<
            ") and alphas (" << alphaMin.size() << ")");

    	const std::vector<Time>& evolutionTimes = evolution.evolutionTimes();
    	std::vector<Time> temp(rateTimes.begin(), rateTimes.end()-1);
    	QL_REQUIRE(temp==evolutionTimes,
                   "mismatch between evolutionTimes and rateTimes");


    	Size numberOfSteps = evolution.numberOfSteps();

        // factor reduction
    	std::vector<Matrix> corrPseudo(corr.times().size());
    	for (Size i=0; i<corrPseudo.size(); ++i)
        	corrPseudo[i] = rankReducedSqrt(corr.correlation(i),
        	numberOfFactors, 1.0,
        	SalvagingAlgorithm::None);

    	alpha.resize(numberOfRates);
    	a.resize(numberOfRates);
    	b.resize(numberOfRates);

    	alpha[0] = alphaInitial[0]; // has no effect on anything in any case
    	a[0] = b[0] = 1.0; // no modifications to swap vol for first rate

    	alphafinder solver(parametricform);

        // vectors for new vol
    	std::vector<std::vector<Volatility> > newVols;
    	std::vector<Volatility> theseNewVols(numberOfRates);

        // get Zinverse, we can get wj later
    	Matrix zedMatrix =
        	SwapForwardMappings::coterminalSwapZedMatrix(cs, displacement);
    	Matrix invertedZedMatrix = inverse(zedMatrix);

    	std::vector<Volatility> rateonevols(numberOfRates);
    	rateonevols[0] = sqrt(displacedSwapVariances[0]->variances()[0]);
    	std::vector<Volatility> ratetwovols(numberOfRates);
    	std::vector<Real> correlations(numberOfRates);
    	newVols.push_back(rateonevols);


    	for (Size i =0; i < numberOfRates-1; ++i) {
            // we will calibrate caplet on forward rate i,
            // we will do this by modifying the vol of swap rate i+1
            // final caplet and swaption are the same, so we skip that case
        	const std::vector<Real>& var =
	                                displacedSwapVariances[i+1]->variances();
        	for (Size j =0; j < i+2; ++j)
            	ratetwovols[j] = sqrt(var[j]);

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
                                                    rateonevols,
                                                    ratetwovols,
                                                    correlations,
                                                    w0,
                                                    w1,
                                                    targetVariance,
                                                    toleranceForAlphaSolving,
                                                    alphaMax[i+1],
                                                    alphaMin[i+1],
                                                    steps,
                                                    alpha[i+1],
                                                    a[i+1],
                                                    b[i+1],
                                                    theseNewVols);
        	else
            	success = solver.solve(alphaInitial[i+1] ,
                                       i,
                                       rateonevols,
                                       ratetwovols,
                                       correlations,
                                       w0,
                                       w1,
                                       targetVariance,
                                       toleranceForAlphaSolving,
                                       alphaMax[i+1],
                                       alphaMin[i+1],
                                       steps,
                                       alpha[i+1],
                                       a[i+1],
                                       b[i+1],
                                       theseNewVols);

        	if (!success)
            	return success; // i.e. false

        	newVols.push_back(theseNewVols);
        	rateonevols = theseNewVols;
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

    	return true;
    }


	bool calibrationOfAlphaFunctionDataIterative(
    	const EvolutionDescription& evolution,
    	const PiecewiseConstantCorrelation& corr,
    	const std::vector<boost::shared_ptr<
    	PiecewiseConstantVariance> >&
    	displacedSwapVariances,
    	const std::vector<Volatility>& capletVols,
    	const CurveState& cs,
    	const Spread displacement,
    	const Size numberOfFactors,
    	boost::shared_ptr<alphaform> parametricform,
    	const std::vector<Real>& alphaInitial,
    	const std::vector<Real>& alphaMax,
    	const std::vector<Real>& alphaMin,
    	Integer steps,
    	Real toleranceForAlphaSolving,
    	bool maximizeHomogeneity,
    	Size maxIterations,
    	Real toleranceForIterativeSolving,
    	std::vector<Real>& alpha, // for info only,
    	std::vector<Real>& a,     // for info only,
    	std::vector<Real>& b,     // for info only,
    	std::vector<Matrix>& swapCovariancePseudoRoots // the real result
        )
    {
    	std::vector<Volatility> modifiedCapletVols=capletVols;
    	Real error;
    	Size iterations=0;
    	Size numberOfRates = evolution.numberOfRates();

    	do {
        	bool success = calibrationOfAlphaFunctionData(
            	evolution,
            	corr,
            	displacedSwapVariances,
            	modifiedCapletVols,
            	cs,
            	displacement,
            	numberOfFactors,
            	parametricform,
            	alphaInitial,
            	alphaMax,
            	alphaMin,
            	steps,
            	toleranceForAlphaSolving,
            	maximizeHomogeneity,
            	alpha,
            	a,
            	b,
            	swapCovariancePseudoRoots);
        	if (!success)
            	return false;

            std::vector<Spread> displacements(evolution.numberOfRates(),
                                              displacement);
        	boost::shared_ptr<MarketModel> smm(new
            	PseudoRootFacade(swapCovariancePseudoRoots,
                                 evolution.rateTimes(),
                                 cs.coterminalSwapRates(),
                                 displacements));

        	CotSwapToFwdAdapter flmm(smm);
        	Matrix capletTotCovariance =
                            flmm.totalCovariance(evolution.numberOfRates()-1);

        	std::vector<Volatility> capletVolsOutput(numberOfRates);
        	for (Size i=0; i<numberOfRates; ++i) {
            	capletVolsOutput[i] = std::sqrt(capletTotCovariance[i][i] /
            	                                evolution.rateTimes()[i]);
            }

        	error=0.0;
            // check caplet fit
        	for (Size i=0; i<numberOfRates-1; ++i) {
            	Real thisError = capletVols[i]-capletVolsOutput[i];
            	error += thisError*thisError;

            	modifiedCapletVols[i] = modifiedCapletVols[i] * capletVols[i] /
                                                            capletVolsOutput[i];
            }
            ++iterations;
        }

    	while (iterations<maxIterations &&
    	       sqrt(error)>toleranceForIterativeSolving);

    	return true;
    }

    bool calibrationOfAlphaFunction(
    	const EvolutionDescription& evolution,
    	const PiecewiseConstantCorrelation& corr,
    	const std::vector<boost::shared_ptr<
    	PiecewiseConstantVariance> >&
    	displacedSwapVariances,
    	const std::vector<Volatility>& capletVols,
    	const CurveState& cs,
    	const Spread displacement,
    	const Size numberOfFactors,
    	boost::shared_ptr<alphaform> parametricform,
    	const std::vector<Real>& alphaInitial,
    	const std::vector<Real>& alphaMax,
    	const std::vector<Real>& alphaMin,
    	Integer steps,
    	Real toleranceForAlphaSolving,
    	Size maxIterations,
    	Real toleranceForIterativeSolving,
    	std::vector<Matrix>& swapCovariancePseudoRoots
        )
    {
    	std::vector<Real> alpha(evolution.numberOfRates());
    	std::vector<Real> a(evolution.numberOfRates());
    	std::vector<Real> b(evolution.numberOfRates());

    	return calibrationOfAlphaFunctionDataIterative(
        	evolution,
        	corr,
        	displacedSwapVariances,
        	capletVols,
        	cs,
        	displacement,
        	numberOfFactors,
        	parametricform,
        	alphaInitial,
        	alphaMax,
        	alphaMin,
        	steps,
        	toleranceForAlphaSolving,
        	true,
            maxIterations,
        	toleranceForIterativeSolving,
        	alpha,
        	a,
        	b,
        	swapCovariancePseudoRoots
            );
    }

}
