
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

/*! \file randomgenerator.cpp
	\brief Uniform random number generator
	
	$Source$
	$Name$
	$Log$
	Revision 1.3  2000/12/20 17:00:59  enri
	modified to use new macros

	Revision 1.2  2000/12/18 18:32:47  lballabio
	Added CVS tags
	
*/


#include "randomgenerator.h"

namespace QuantLib {

	namespace Math {
	
		const long RandomGenerator::m1 = 2147483563L;
		const long RandomGenerator::a1 = 40014L;
		const long RandomGenerator::q1 = 53668L;
		const long RandomGenerator::r1 = 12211L;
		
		const long RandomGenerator::m2 = 2147483399L;
		const long RandomGenerator::a2 = 40692L;
		const long RandomGenerator::q2 = 52774L;
		const long RandomGenerator::r2 = 3791L;
		
		const int RandomGenerator::bufferSize = 32;
		const long RandomGenerator::bufferNormalizer = 67108862L; 			// 1+(m1-1)/bufferSize

		const long double RandomGenerator::maxRandom = 1.0-QL_EPSILON;
	
	}
}
