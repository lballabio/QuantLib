
/*
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "bsmoption.h"
#include "brent.h"
#include "qlerrors.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Pricers)

double BSMOption::impliedVolatility(double targetValue, double accuracy, int maxEvaluations) const {
	// check option targetValue boundary condition
	Require(targetValue>=0.0, "BSMOption::impliedVol : negative option targetValue not allowed");
  // the following checks may be improved
 	switch (theType) {
	  case Option::Call:
		Require(targetValue <= theUnderlying, "BSMOption::impliedVol : call option targetValue (" + DoubleFormat(targetValue) +
		  ") > underlying value (" + DoubleFormat(theUnderlying) + ") not allowed");
		break;
	  case Option::Put:
		Require(targetValue <= theStrike, "BSMOption::impliedVol : put option targetValue (" + DoubleFormat(targetValue) +
		  ") > strike value (" + DoubleFormat(theStrike) + ") not allowed");
		break;
	  case Option::Straddle:
		// to be verified
		Require(targetValue < theUnderlying+theStrike,
		  "BSMOption::impliedFlatVol : straddle option targetValue (" + DoubleFormat(targetValue) +
		  ") >= (underlying+strike) value (" + DoubleFormat(theUnderlying+theStrike) + ") not allowed");
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
	s1d.setLowBound(DBL_EPSILON);

	return s1d.solve(bsmf, accuracy, theVolatility, 0.05);

}

QL_END_NAMESPACE(Pricers)

QL_END_NAMESPACE(QuantLib)