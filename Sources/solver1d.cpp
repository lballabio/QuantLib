
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

// $Source$

// $Log$
// Revision 1.8  2000/12/13 18:10:41  nando
// CVS keyword added
//

#include "solver1d.h"

namespace QuantLib {

	const double growthFactor = 1.6;
	
	double Solver1D::solve(const ObjectiveFunction& f, double xAccuracy, double guess, double step) const {
	
	  int flipflop = -1;
	
	  root = guess;
	  fxMax = f.value(root);
	
	  // monotonically crescent bias, as in optionValue(volatility)
	  if (fxMax == 0.0)
		return root;
	  else if (fxMax > 0.0) {
	    xMin = enforceBounds(root-step);
	    fxMin = f.value(xMin);
	    xMax = root;
	  } else {
		  xMin = root;
		  fxMin = fxMax;
		  xMax = enforceBounds(root+step);
		  fxMax = f.value(xMax);
	  }
	
	  evaluationNumber=2;
	  while (evaluationNumber<=maxEvaluations) {
		  if (fxMin*fxMax <= 0.0) {
		    if (fxMin == 0.0)    return xMin;
		    if (fxMax == 0.0)    return xMax;
		    root = (xMax+xMin)/2.0;
			return _solve(f, QL_MAX(QL_FABS(xAccuracy), std::numeric_limits<double>::epsilon()));
		  }
		  if (QL_FABS(fxMin) < QL_FABS(fxMax)) {
		    xMin = enforceBounds(xMin+growthFactor*(xMin-xMax));
		    fxMin=f.value(xMin);
		  } else if (QL_FABS(fxMin) > QL_FABS(fxMax)) {
		    xMax = enforceBounds(xMax+growthFactor*(xMax-xMin));
		    fxMax=f.value(xMax);
		  } else if (flipflop == -1) {
		    xMin = enforceBounds(xMin+growthFactor*(xMin-xMax));
		    fxMin=f.value(xMin);
		    evaluationNumber++;
		    flipflop = 1;
		  } else if (flipflop == 1) {
		    xMax = enforceBounds(xMax+growthFactor*(xMax-xMin));
		    fxMax=f.value(xMax);
		    flipflop = -1;
		  } 
		  evaluationNumber++;
	  }
	
	  throw Error("unable to bracket root in " + IntegerFormatter::toString(maxEvaluations) +
	    " function evaluations (last bracket attempt: f[" + DoubleFormatter::toString(xMin) +
		  "," + DoubleFormatter::toString(xMax) + "] -> [" +
		  DoubleFormatter::toString(fxMin) + "," + DoubleFormatter::toString(fxMax) + "])");
	}
	
	
	double Solver1D::solve(const ObjectiveFunction& f, double xAccuracy, double guess, double xMin_, double xMax_) const {
	
	  xMin = xMin_;
	  xMax = xMax_;
	  Require(xMin < xMax, "invalid range: xMin (" + DoubleFormatter::toString(xMin)
	  	+ ") >= xMax (" + DoubleFormatter::toString(xMax) + ")");
	
	  Require(!lowBoundEnforced || xMin >= theLowBound, "xMin (" + DoubleFormatter::toString(xMin) +
	    ") < enforced low bound (" + DoubleFormatter::toString(theLowBound) + ")");
	  Require(!hiBoundEnforced || xMax <= theHiBound, "xMax (" + DoubleFormatter::toString(xMax) +
	    ") > enforced hi bound (" + DoubleFormatter::toString(theHiBound) + ")");
	
	  fxMin = f.value(xMin);
	  if (fxMin == 0.0)    return xMin;
	  fxMax = f.value(xMax);
	  if (fxMax == 0.0)    return xMax;
	  evaluationNumber = 2;
	
	  Require((fxMin*fxMax < 0.0),
	  	"root not bracketed: f["+DoubleFormatter::toString(xMin)+","+DoubleFormatter::toString(xMax)+"] -> ["
		  +DoubleFormatter::toString(fxMin)+","+DoubleFormatter::toString(fxMax)+"]");
	
	
	  Require(guess > xMin, "Solver1D: guess (" + DoubleFormatter::toString(guess) + 
		  ") < xMin (" + DoubleFormatter::toString(xMin) + ")");
	  Require(guess < xMax, "Solver1D: guess (" + DoubleFormatter::toString(guess) +
		  ") > xMax (" + DoubleFormatter::toString(xMax) + ")");
	  root = guess;
	
	  return _solve(f, QL_MAX(QL_FABS(xAccuracy), std::numeric_limits<double>::epsilon()));
	}

}
