
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

// The implementation of the algorithm was inspired from
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "newtonsafe.h"

QL_USING(QuantLib, Error)
QL_USING(QuantLib, IntegerFormat)
QL_USING(QuantLib,Require)
QL_USING(QuantLib,IsNull)

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solvers1D)

double NewtonSafe::_solve(const Function& f, double xAccuracy) const {

	double froot, dfroot, dx, dxold;
	double xh, xl;

	if (fxMin < 0.0) {                    // Orient the search so that f(xl) < 0.
		xl=xMin;
		xh=xMax;
	} else {
		xh=xMin;
		xl=xMax;
	}

	dxold=xMax-xMin;  // the "stepsize before last"
 										// it was dxold=fabs(xMax-xMin); in Numerical Recipes
										// here (xMax-xMin > 0) is verified in the constructor
	
	dx=dxold;         // and the last step.

	froot = f.value(root);
	dfroot = f.derivative(root);
	Require(!IsNull(dfroot), "NewtonSafe requires function's derivative");
	evaluationNumber++;

	while (evaluationNumber<=maxEvaluations) {
		if ((((root-xh)*dfroot-froot)*((root-xl)*dfroot-froot) > 0.0)     // Bisect if Newton out of range,
			|| (QL_FABS(2.0*froot) > QL_FABS(dxold*dfroot))) {                    //or not decreasing fast enough.
			dxold = dx;
			dx = (xh-xl)/2.0;
			root=xl+dx;
		} else {
			dxold=dx;
			dx=froot/dfroot;
			root -= dx;
		}
	if (QL_FABS(dx) < xAccuracy) // Convergence criterion.
		return root;
	froot = f.value(root);
	dfroot = f.derivative(root);
	evaluationNumber++;
	if (froot < 0.0)
		xl=root;
	else
		xh=root;
	}

	throw Error("NewtonSafe: "
	"maximum number of function evaluations ("
	+ IntegerFormat(maxEvaluations) + ") exceeded");
	return 0.0;
}

QL_END_NAMESPACE(Solvers1D)

QL_END_NAMESPACE(QuantLib)
