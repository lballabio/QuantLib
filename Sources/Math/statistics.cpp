
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

/*! \file statistics.cpp
	\brief statistic tools
	
	$Source$
	$Name$
	$Log$
	Revision 1.6  2001/01/12 18:28:44  lballabio
	Updated to reflect changes in statistics.h

	Revision 1.5  2000/12/20 15:30:21  lballabio
	Using new defines for helping Linux port
	
	Revision 1.4  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "statistics.h"

namespace QuantLib {

	namespace Math {
		
		Statistics::Statistics() {
			reset();
		}
		
		void Statistics::reset() {
			min_ = QL_MAX_DOUBLE;
			max_ = QL_MIN_DOUBLE;
			sampleNumber_ = 0;
			sampleWeight_ = 0.0;
			sum_ = 0.0;
			quadraticSum_ = 0.0;
			cubicSum_ = 0.0;
			fourthPowerSum_ = 0.0;
		}

	}

}
