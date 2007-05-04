//
//
//											alphaformconcrete.h
//
//

#ifndef alpha_form_concrete
#define alpha_form_concrete
#include <ql/models/marketmodels/models/alphaform.hpp>
#include <vector>

namespace QuantLib
{
class alphaforminverselinear : public alphaform
{
public:

	alphaforminverselinear( const std::vector<Time>& times,
												Real alpha =0.0);
	virtual ~alphaforminverselinear();

	virtual Real operator()(Integer i) const;

	virtual void setAlpha(Real alpha_);

private:
	std::vector<Time> times_;
	Real alpha_;

};


class alphaformlinearhyperbolic : public alphaform
{
public:

	alphaformlinearhyperbolic( const std::vector<Time>& times,
												Real alpha =0.0);
	virtual ~alphaformlinearhyperbolic();

	virtual Real operator()(Integer i) const;

	virtual void setAlpha(Real alpha_);

private:
	std::vector<Time> times_;
	Real alpha_;

};
}
#endif
