
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License: see the file LICENSE.TXT for details.
 * Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file.
 * LICENCE.TXT is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/


// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "newtonsafe.h"

namespace QuantLib {

	namespace Solvers1D {
	
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
		
			throw Error("NewtonSafe: maximum number of function evaluations ("
			  + IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
