
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "blackscholesmerton.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Operators)

BSMOperator::BSMOperator(int size, double dx, double r, double q, double sigma)
: TridiagonalOperator(size) {
	double sigma2 = sigma*sigma;
	double nu = r-q-sigma2/2;
	double pd = -(sigma2/dx-nu)/(2*dx);
	double pm = sigma2/(dx*dx)-r;
	double pu = -(sigma2/dx+nu)/(2*dx);
	setMidRows(pd,pm,pu);
}


QL_END_NAMESPACE(Operators)

QL_END_NAMESPACE(QuantLib)
