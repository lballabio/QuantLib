
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

/*! \file bsmoption.cpp
	\brief common code for option evaluation
	
	$Source$
	$Name$
	$Log$
	Revision 1.15  2000/12/20 17:00:59  enri
	modified to use new macros

	Revision 1.14  2000/12/14 12:40:14  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "bsmoption.h"
#include "brent.h"

namespace QuantLib {

	namespace Pricers {
	
		double BSMOption::impliedVolatility(double targetValue, double accuracy, int maxEvaluations) const {
			// check option targetValue boundary condition
			Require(targetValue>=0.0, "BSMOption::impliedVol : negative option targetValue not allowed");
			// the following checks may be improved
		 	switch (theType) {
			  case Call:
				QuantLib::Require(targetValue <= theUnderlying, 
				  "BSMOption::impliedVol : call option targetValue (" + DoubleFormatter::toString(targetValue) +
				  ") > underlying value (" + DoubleFormatter::toString(theUnderlying) + ") not allowed");
				break;
			  case Put:
				QuantLib::Require(targetValue <= theStrike, 
				  "BSMOption::impliedVol : put option targetValue (" + DoubleFormatter::toString(targetValue) +
				  ") > strike value (" + DoubleFormatter::toString(theStrike) + ") not allowed");
				break;
			  case Straddle:
				// to be verified
				QuantLib::Require(targetValue < theUnderlying+theStrike,
				  "BSMOption::impliedFlatVol : straddle option targetValue (" + DoubleFormatter::toString(targetValue) +
				  ") >= (underlying+strike) value (" + DoubleFormatter::toString(theUnderlying+theStrike) + ") not allowed");
				break;
			  default:
				throw IllegalArgumentError("BSMOption: invalid option type");
			}
			// clone used for root finding
			Handle<BSMOption> tempBSM = clone();
			// objective function
			BSMFunction bsmf(tempBSM, targetValue);		
			// solver
			Solvers1D::Brent s1d = Solvers1D::Brent();
			s1d.setMaxEvaluations(maxEvaluations);
			s1d.setLowBound(QL_EPSILON);
		
			return s1d.solve(bsmf, accuracy, theVolatility, 0.05);
		}
	
	}

}
