
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "falseposition.h"

namespace QuantLib {

	namespace Solvers1D {
	
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
			  + IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
