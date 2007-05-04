//
//
//											alphaform.h
//
//

#ifndef alpha_form
#define alpha_form
#include <ql/types.hpp>


namespace QuantLib {

class alphaform
{
public:
	alphaform();
	virtual ~alphaform();

	virtual Real operator()(Integer i) const=0;

	virtual void setAlpha(Real alpha)=0;

};
}
#endif
