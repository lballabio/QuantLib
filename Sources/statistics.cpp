
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "statistics.h"
#include <limits>


QL_BEGIN_NAMESPACE(QuantLib)

Statistics::Statistics() {
  theMin = std::numeric_limits<double>::min();
  theMax = std::numeric_limits<double>::max();
  theSampleNumber = 0;
  theSampleWeight = 0.0;
  theSum = 0.0;
  theQuadraticSum = 0.0;
  theCubicSum = 0.0;
  theFourthPowerSum = 0.0;
}

QL_END_NAMESPACE(QuantLib)
