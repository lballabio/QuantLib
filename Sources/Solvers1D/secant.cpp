
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

#include "secant.h"

namespace QuantLib {

	namespace Solvers1D {
	
		double Secant::_solve(const ObjectiveFunction& f, double xAccuracy) const {
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
