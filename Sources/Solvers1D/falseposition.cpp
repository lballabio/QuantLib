
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "falseposition.h"

QL_USING(QuantLib,Error)
QL_USING(QuantLib,IntegerFormat)

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solvers1D)

double FalsePosition::_solve(const Function& f, double xAccuracy) const {

  double fl,fh,xl,xh,dx,del,froot;

  if (fxMin < 0.0) { // Identify the limits so that xl corresponds to the low side.
		xl=xMin;
  	fl = fxMin;
		xh=xMax;
  	fh = fxMax;
	} else {
		xl=xMax;
  	fl = fxMax;
		xh=xMin;
  	fh = fxMin;
	}
	dx=xh-xl;
	while (evaluationNumber<=maxEvaluations) {  // False position loop
		root=xl+dx*fl/(fl-fh);              // Increment with respect to latest value.
		froot=f.value(root);
  	evaluationNumber++;
		if (froot < 0.0) {                      // Replace appropriate limit.
			del=xl-root;
			xl=root;
			fl=froot;
		} else {
			del=xh-root;
			xh=root;
			fh=froot;
		}
		dx=xh-xl;
  	if (QL_FABS(del) < xAccuracy || froot == 0.0)  { // Convergence criterion.
	  	return root;
  	}
	}
	throw Error("FalsePosition: maximum number of function evaluations ("
	  + IntegerFormat(maxEvaluations) + ") exceeded");
	QL_DUMMY_RETURN(0.0);
}

QL_END_NAMESPACE(Solvers1D)

QL_END_NAMESPACE(QuantLib)
