
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

namespace QuantLib {

	namespace Solvers1D {
	
		#define SIGN(a,b) ((b) >= 0.0 ? QL_FABS(a) : -QL_FABS(a))
		
		double Brent::_solve(const ObjectiveFunction& f, double xAccuracy) const {
		 	double d,e,min1,min2;
			double froot,p,q,r,s,xAcc1,xMid;
		
			root=xMax;
			froot=fxMax;
			while (evaluationNumber<=maxEvaluations) {
				if ((froot > 0.0 && fxMax > 0.0) || (froot < 0.0 && fxMax < 0.0)) {
				// Rename xMin, root, xMax and adjust bounding interval d
					xMax=xMin;
					fxMax=fxMin;
					e=d=root-xMin;
				}
				if (QL_FABS(fxMax) < QL_FABS(froot)) {
					xMin=root;
					root=xMax;
					xMax=xMin;
					fxMin=froot;
					froot=fxMax;
					fxMax=fxMin;
				}
				xAcc1=2.0*std::numeric_limits<double>::epsilon()*QL_FABS(root)+0.5*xAccuracy; // Convergence check.
				xMid=(xMax-root)/2.0;
				if (QL_FABS(xMid) <= xAcc1 || froot == 0.0)		return root;
				if (QL_FABS(e) >= xAcc1 && QL_FABS(fxMin) > QL_FABS(froot)) {
					s=froot/fxMin;                   // Attempt inverse quadratic interpolation.
					if (xMin == xMax) {
						p=2.0*xMid*s;
						q=1.0-s;
					} else {
							q=fxMin/fxMax;
							r=froot/fxMax;
							p=s*(2.0*xMid*q*(q-r)-(root-xMin)*(r-1.0));
							q=(q-1.0)*(r-1.0)*(s-1.0);
					}
					if (p > 0.0) q = -q;  // Check whether in bounds.
					p=QL_FABS(p);
					min1=3.0*xMid*q-QL_FABS(xAcc1*q);
					min2=QL_FABS(e*q);
					if (2.0*p < (min1 < min2 ? min1 : min2)) {
						e=d;                // Accept interpolation.
						d=p/q;
					} else {
						d=xMid;             // Interpolation failed, use bisection.
						e=d;
					}
				} else {                // Bounds decreasing too slowly, use bisection.
					d=xMid;
					e=d;
				}
				xMin=root;
				fxMin=froot;
				if (QL_FABS(d) > xAcc1)
					root += d;
				else
					root += SIGN(xAcc1,xMid);
				froot=f.value(root);
				evaluationNumber++;
			}
			throw Error("Brent: maximum number of function evaluations ("
			+ IntegerFormatter::toString(maxEvaluations) + ") exceeded");
			QL_DUMMY_RETURN(0.0);
		}
	
	}

}
