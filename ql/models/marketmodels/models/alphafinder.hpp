//
//
//												alphafinder.hpp
//
//

#ifndef alpha_finder_h
#define alpha_finder_h

#include  <ql/models/marketmodels/models/alphaform.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace QuantLib
{

class alphafinder
{
public:
	alphafinder(boost::shared_ptr<alphaform> parametricform);

	bool solve( Real alpha0,
						Integer stepindex,
						const std::vector<Volatility>& rateonevols,
						const std::vector<Volatility>& ratetwohomogeneousvols,
						const std::vector<Real>& correlations,
						Real w0,
						Real w1,
						Real targetVariance,			
    					Real tolerance,
    					Real alphaMax,
	 					Real alphaMin,
						Integer steps,
						Real& alpha,
						Real& a,
						Real& b,
						std::vector<Volatility>& ratetwovols
						);

	bool solveWithMaxHomogeneity( Real alpha0,
						Integer stepindex,
						const std::vector<Volatility>& rateonevols,
						const std::vector<Volatility>& ratetwohomogeneousvols,
						const std::vector<Real>& correlations,
						Real w0,
						Real w1,
						Real targetVariance,			
    					Real tolerance,
    					Real alphaMax,
	 					Real alphaMin,
						Integer steps,
						Real& alpha,
						Real& a,
						Real& b,
						std::vector<Volatility>& ratetwovols
						);


private:
	
	Real computeLinearPart(
									Real alpha
									);

	Real computeQuadraticPart(
									Real alpha
									);

	Real valueAtTurningPoint(Real alpha);
	Real minusValueAtTurningPoint(Real alpha);
	bool testIfSolutionExists(Real alpha);

	bool finalPart( Real alphaFound,
						Integer stepindex,
						const std::vector<Volatility>& ratetwohomogeneousvols,
						Real quadraticPart,
						Real linearPart,
						Real constantPart,
						Real& alpha,
						Real& a,
						Real& b,
						std::vector<Volatility>& ratetwovols);

	
	Real alphafinder::homogeneityfailure(Real alpha);


	boost::shared_ptr<alphaform> parametricform_;


	Integer stepindex_;
	std::vector<Volatility> rateonevols_;
	std::vector<Volatility>	ratetwohomogeneousvols_;
	std::vector<Volatility> putativevols_;
	std::vector<Real> correlations_;
	Real	 w0_;
	Real	 w1_;
	Real constantPart_;
	Real linearPart_;
	Real quadraticPart_;
	Real totalVar_;
	Real targetVariance_;


};

}

#endif

