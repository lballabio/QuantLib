
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef ql_solver1d_newtonsafe_h
#define ql_solver1d_newtonsafe_h

#include "solver1d.h"


QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solver1D)

class NewtonSafe : public Solver1D {
	public:
	NewtonSafe() {}
	private:
	double _solve(const Function& f, double xAccuracy) const;
};

QL_END_NAMESPACE(Solver1D)

QL_END_NAMESPACE(QuantLib)

#endif
