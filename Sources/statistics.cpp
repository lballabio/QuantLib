
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "statistics.h"
#include <limits>


namespace QuantLib {

	Statistics::Statistics() {
	  reset();
	}
	
	void Statistics::reset() {
	  theMin = std::numeric_limits<double>::max();
	  theMax = std::numeric_limits<double>::min();
	  theSampleNumber = 0;
	  theSampleWeight = 0.0;
	  theSum = 0.0;
	  theQuadraticSum = 0.0;
	  theCubicSum = 0.0;
	  theFourthPowerSum = 0.0;
	}

}
