
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "bisection.h"

namespace QuantLib {

	namespace Solvers1D {
	
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
			throw Error("Bisection: maximum number of function evaluations ("
			  + IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
