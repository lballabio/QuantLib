
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "blackscholesmerton.h"

namespace QuantLib {

	namespace Operators {
	
		BSMOperator::BSMOperator(int size, double dx, double r, double q, double sigma)
		: TridiagonalOperator(size) {
			double sigma2 = sigma*sigma;
			double nu = r-q-sigma2/2;
			double pd = -(sigma2/dx-nu)/(2*dx);
			double pu = -(sigma2/dx+nu)/(2*dx);
			double pm = sigma2/(dx*dx)+r;
			setMidRows(pd,pm,pu);
		}
	
	}

}
