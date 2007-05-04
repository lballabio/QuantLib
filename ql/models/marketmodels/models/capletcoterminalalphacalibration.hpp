//
//
//										CapletCoterminalAlphaCalibration.hpp
//
//

#ifndef CAPLET_COTERMINAL_ALPHA_CALIBRATION
#define CAPLET_COTERMINAL_ALPHA_CALIBRATION
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/piecewiseconstantcorrelation.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class PiecewiseConstantVariance;
    class Matrix;
	class alphaform;

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
    );

// doesn't return as much for those not interested in all the extras
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
    );
}

#endif
