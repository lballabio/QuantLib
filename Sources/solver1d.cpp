
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "solver1d.h"

QL_USING(QuantLib, Error)
QL_USING(QuantLib, IntegerFormat)
QL_USING(QuantLib, Require)
QL_USING(QuantLib, DoubleFormat)

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Solver1D)

const double growthFactor = 1.6;

double Solver1D::solve(const Function& f, double xAccuracy, double guess, double step) const {

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
  	  return _solve(f, QL_MAX(QL_FABS(xAccuracy), DBL_EPSILON));
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

  throw Error("unable to braket root in " + IntegerFormat(maxEvaluations) +
    " function evaluations (last bracket attempt: f[" + DoubleFormat(xMin) +
	  "," + DoubleFormat(xMax) + "] -> [" +
	  DoubleFormat(fxMin) + "," + DoubleFormat(fxMax) + "])");
}


double Solver1D::solve(const Function& f, double xAccuracy, double guess, double xMin_, double xMax_) const {

  xMin = xMin_;
  xMax = xMax_;
  // better safe than sorry
  Require(xMin < xMax, "invalid range: xMin (" + DoubleFormat(xMin) + ") >= xMax (" + DoubleFormat(xMax) + ")");

  Require(!lowBoundEnforced || xMin >= theLowBound, "xMin (" + DoubleFormat(xMin) +
    ") < enforced low bound (" + DoubleFormat(theLowBound) + ")");
  Require(!hiBoundEnforced || xMax <= theHiBound, "xMax (" + DoubleFormat(xMax) +
    ") > enforced hi bound (" + DoubleFormat(theHiBound) + ")");

  fxMin = f.value(xMin);
  if (fxMin == 0.0)    return xMin;
  fxMax = f.value(xMax);
  if (fxMax == 0.0)    return xMax;
  evaluationNumber = 2;

  Require((fxMin*fxMax < 0.0),
  	"root not bracketed: f["+DoubleFormat(xMin)+","+DoubleFormat(xMax)+"] -> ["
	  +DoubleFormat(fxMin)+","+DoubleFormat(fxMax)+"]");


  Require(guess > xMin, "Solver1D: guess (" + DoubleFormat(guess) + 
	  ") < xMin (" + DoubleFormat(xMin) + ")");
  Require(guess < xMax, "Solver1D: guess (" + DoubleFormat(guess) +
	  ") > xMax (" + DoubleFormat(xMax) + ")");
  root = guess;

  return _solve(f, QL_MAX(QL_FABS(xAccuracy), DBL_EPSILON));
}

QL_END_NAMESPACE(Solver1D)

QL_END_NAMESPACE(QuantLib)
