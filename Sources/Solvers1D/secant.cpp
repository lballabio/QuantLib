
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "secant.h"

namespace QuantLib {

	namespace Solvers1D {
	
		double Secant::_solve(const Function& f, double xAccuracy) const {
			double fl,froot,dx,xl;
		
			if (QL_FABS(fxMin) < QL_FABS(fxMax)) { // Pick the bound with the smaller function value as the most recent guess.
				root=xMin;
				froot=fxMin;
				xl=xMax;
				fl=fxMax;
			} else {
				root=xMax;
				froot=fxMax;
				xl=xMin;
				fl=fxMin;
			}
			while (evaluationNumber<=maxEvaluations) {
				dx=(xl-root)*froot/(froot-fl);
				xl=root;
				fl=froot;
				root += dx;
				froot=f.value(root);
		  	evaluationNumber++;
				if (QL_FABS(dx) < xAccuracy || froot == 0.0)  return root;
			}
			throw Error("Secant: maximum number of function evaluations ("
			  + IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
		
	}

}
