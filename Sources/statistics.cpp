
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
