
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bisection.h"

QL_USING(QuantLib, Error)
QL_USING(QuantLib, IntegerFormat)

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solvers1D)

double Bisection::_solve(const Function& f, double xAccuracy) const {
	double dx,xMid,fMid;

	if (fxMin < 0.0) { // Orient the search so that f>0
	dx = xMax-xMin;  // lies at root+dx.
	root = xMin;
	} else {
	dx = xMin-xMax;
	root = xMax;
	}

	while (evaluationNumber<=maxEvaluations) {
		dx /= 2.0;
	xMid=root+dx;
	fMid=f.value(xMid);
	evaluationNumber++;
		if (fMid <= 0.0) 
		root=xMid;
		if (QL_FABS(dx) < xAccuracy || fMid == 0.0) {
		return root;
	}
	}
	throw Error("Bisection: "
	"maximum number of function evaluations ("
	+ IntegerFormat(maxEvaluations) + ") exceeded");
	return 0.0;
}

QL_END_NAMESPACE(Solvers1D)

QL_END_NAMESPACE(QuantLib)
