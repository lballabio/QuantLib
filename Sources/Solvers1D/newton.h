
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_solver1d_newton_h
#define quantlib_solver1d_newton_h

#include "solver1d.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solvers1D)

class Newton : public Solver1D {
	public:
	Newton() {}
	private:
	double _solve(const Function& f, double xAccuracy) const;
};

QL_END_NAMESPACE(Solvers1D)

QL_END_NAMESPACE(QuantLib)

#endif
