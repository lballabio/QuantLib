
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "newton.h"
#include "newtonsafe.h"

namespace QuantLib {

	namespace Solvers1D {
	
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
		
			throw Error("Newton: maximum number of function evaluations ("
			  + IntegerFormat(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
