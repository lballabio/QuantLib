//
//
//										CapletCoterminalAlphaCalibration.cpp
//
//


#include <ql/models/marketmodels/models/capletcoterminalalphacalibration.hpp>
#include <ql/models/marketmodels/models/alphafinder.hpp>
#include <ql/models/marketmodels/models/piecewiseconstantvariance.hpp>
#include <ql/models/marketmodels/models/pseudorootfacade.hpp>
#include <ql/models/marketmodels/models/cotswaptofwdadapter.hpp>
#include <ql/models/marketmodels/swapforwardmappings.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>

namespace QuantLib {
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
                            std::vector<Matrix>& swapCovariancePseudoRoots
    )
{
	std::vector<Real> alpha(evolution.numberOfRates());
	std::vector<Real> a(evolution.numberOfRates());
	std::vector<Real> b(evolution.numberOfRates());
	

	return calibrationOfAlphaFunctionData(
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
							alpha,
							a,
							b,
                            swapCovariancePseudoRoots
    );
}
	
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
							std::vector<Real>& alpha,
							std::vector<Real>& a,
							std::vector<Real>& b,
                            std::vector<Matrix>& swapCovariancePseudoRoots
    )
{
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
        QL_REQUIRE(std::vector<Time>(rateTimes.begin(), rateTimes.end()-1)==evolutionTimes,
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
	

		for (Size i =0; i < numberOfRates-1; ++i)
		{ // we will calibrate caplet on forward rate i,
		  // we will do this by modifying the vol of swap rate i+1
			// final caplet and swaption are the same, so we skip that case

		  const std::vector<Real>& var = displacedSwapVariances[i+1]->variances();
		  for (Size j =0; j < i+2; ++j)
			  ratetwovols[j] = sqrt(var[j]);

		  for (Size k=0; k < i+1; k++)
		  {
				Real correlation=0.0;
				for (Size l=0; l < numberOfFactors; ++l)
				{	
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

		 solver.solveWithMaxHomogeneity(alphaInitial[i+1] ,
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

}

