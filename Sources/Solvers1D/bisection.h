
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_solver1d_bisection_h
#define quantlib_solver1d_bisection_h

#include "qldefines.h"
#include "solver1d.h"

namespace QuantLib {

	namespace Solvers1D {
	
		class Bisection : public Solver1D {
		  public:
			Bisection() : Solver1D() {}
		  protected:
			double _solve(const Function& f, double xAccuracy) const;
		};
	
	}

}

#endif
