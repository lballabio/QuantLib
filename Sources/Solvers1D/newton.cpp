
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

/*! \file newton.cpp
	\brief Newton 1-D solver
	
	$Source$
	$Name$
	$Log$
	Revision 1.14  2000/12/27 14:05:57  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

	Revision 1.13  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

// The implementation of the algorithm was inspired by
// "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
// Chapter 9

#include "newton.h"
#include "newtonsafe.h"

namespace QuantLib {

	namespace Solvers1D {
	
		double Newton::_solve(const ObjectiveFunction& f, double xAccuracy) const {
			double froot, dfroot, dx;
		
			froot = f.value(root);
			dfroot = f.derivative(root);
			QL_REQUIRE(!IsNull(dfroot), "Newton requires function's derivative");
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
			  + IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
