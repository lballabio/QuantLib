
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

#include "solvers1d.h"
#include <limits>

namespace QuantLib {

	namespace Solvers1D {
	
		#define SIGN(a,b) ((b) >= 0.0 ? QL_FABS(a) : -QL_FABS(a))
		
		double Ridder::_solve(const ObjectiveFunction& f, double xAcc) const {
			double fxMid, froot, s, xMid, nextRoot;
		
			// test on black scholes implied vol show that Ridder solver
			// algorythm actually provides an accuracy 100 times below promised
			double xAccuracy = xAcc/100.0;
		
			root=std::numeric_limits<double>::min();       // Any highly unlikely value, to simplify logic below
		
			while (evaluationNumber<=maxEvaluations) {
				xMid=0.5*(xMin+xMax);
				fxMid=f.value(xMid);    // First of two function evaluations per iteraton.
		  	evaluationNumber++;
				s=QL_SQRT(fxMid*fxMid-fxMin*fxMax);
				if (s == 0.0) return root;
				nextRoot=xMid+(xMid-xMin)*((fxMin >= fxMax ? 1.0 : -1.0)*fxMid/s); // Updating formula.
				if (QL_FABS(nextRoot-root) <= xAccuracy) return root;
		
		    root=nextRoot;
			  froot=f.value(root); // Second of two function evaluations per iteration
			  evaluationNumber++;
			  if (froot == 0.0) return root;
		
			  if (SIGN(fxMid,froot) != fxMid) {  // Bookkeeping to keep the root bracketed on next iteration
				  xMin=xMid;
				  fxMin=fxMid;
				  xMax=root;
				  fxMax=froot;
			  } else if (SIGN(fxMin,froot) != fxMin) {
				  xMax=root;
				  fxMax=froot;
			  } else if (SIGN(fxMax,froot) != fxMax) {
				  xMin=root;
				  fxMin=froot;
			  } else throw Error("Ridder: never get here.");
			  if (QL_FABS(xMax-xMin) <= xAccuracy) return root;
			}
			throw Error("Ridder: maximum number of function evaluations ("
			+ IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
