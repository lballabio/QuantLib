//
//
//										alphaformconcrete.cpp
//
//

#include <ql/models/marketmodels/models/alphaformconcrete.hpp>
#include <cmath>

namespace QuantLib
{

alphaforminverselinear::alphaforminverselinear( const std::vector<Time>& times,
												Real alpha )
												:
												times_(times),
												alpha_(alpha)
{
}
alphaforminverselinear::~alphaforminverselinear()
{
}

Real alphaforminverselinear::operator()(Integer i) const
{
	return 1.0/(1.0+alpha_*times_[i]);
}

void alphaforminverselinear::setAlpha(Real alpha)
{
	alpha_=alpha;
}


alphaformlinearhyperbolic::alphaformlinearhyperbolic( const std::vector<Time>& times,
												Real alpha )
												:
												times_(times),
												alpha_(alpha)
{

}

alphaformlinearhyperbolic::~alphaformlinearhyperbolic()
{

}

Real alphaformlinearhyperbolic::operator()(Integer i) const
{
	Real at = alpha_*times_[i];
	Real res = std::atan(at)-0.5*M_PI;
	res *= at;
	res += 1.0;
	res =std::sqrt(res);
	return res;

}

void alphaformlinearhyperbolic::setAlpha(Real alpha)
{
	alpha_ = alpha;
}



}
