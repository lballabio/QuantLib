
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

#include "normaldistribution.h"

namespace QuantLib {

	namespace Math {
	
		const double NormalDistribution::pi = 3.14159265358979323846;
		
		// For the following formula see M. Abramowitz and I. Stegun, Handbook of Mathematical Functions, 
		// Dover Publications, New York (1972)
		
		const double CumulativeNormalDistribution::a1 = 0.319381530;
		const double CumulativeNormalDistribution::a2 = -0.356563782;
		const double CumulativeNormalDistribution::a3 = 1.781477937;
		const double CumulativeNormalDistribution::a4 = -1.821255978;
		const double CumulativeNormalDistribution::a5 = 1.330274429;
		const double CumulativeNormalDistribution::gamma = 0.2316419;
		const double CumulativeNormalDistribution::precision = 1e-6;
		
		double CumulativeNormalDistribution::operator()(double x) {
			if (x >= 0.0) {
				double k = 1.0/(1.0+gamma*x);
				double temp = gaussian(x)*k*(a1+k*(a2+k*(a3+k*(a4+k*a5))));
				if (temp<precision) return 1.0;
				temp = 1.0-temp;
				if (temp<precision) return 0.0;
				return temp;
			} else {
				return 1.0-(*this)(-x);
			}
		}
		
	}

}
