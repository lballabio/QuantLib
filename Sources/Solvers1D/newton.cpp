
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "newton.h"
#include "newtonsafe.h"

QL_USING(QuantLib, Error)
QL_USING(QuantLib, IntegerFormat)
QL_USING(QuantLib,Require)
QL_USING(QuantLib,IsNull)

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solvers1D)

double Newton::_solve(const Function& f, double xAccuracy) const {
	double froot, dfroot, dx;

	froot = f.value(root);
	dfroot = f.derivative(root);
	Require(!IsNull(dfroot), "Newton requires function's derivative");
	evaluationNumber++;

	while (evaluationNumber<=maxEvaluations) {
		dx=froot/dfroot;
		root -= dx;
    if ((xMin-root)*(root-xMax) < 0.0) { // jumped out of brackets, switch to NewtonSafe
	    NewtonSafe helper;
	    helper.setMaxEvaluations(maxEvaluations-evaluationNumber);
	    return helper.solve(f, xAccuracy, root+dx, xMin, xMax);
    }
    if (QL_FABS(dx) < xAccuracy)  return root;
    froot = f.value(root);
    dfroot = f.derivative(root);
    evaluationNumber++;
	}

	throw Error("Newton: "
	"maximum number of function evaluations ("
	+ IntegerFormat(maxEvaluations) + ") exceeded");
	return 0.0;
}

QL_END_NAMESPACE(Solvers1D)

QL_END_NAMESPACE(QuantLib)
