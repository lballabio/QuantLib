
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

#include "finitedifferences.h"

namespace QuantLib {

	namespace FiniteDifferences {
	
		BSMOperator::BSMOperator(int size, double dx, double r, double q, double sigma)
		: TridiagonalOperator(size) {
			double sigma2 = sigma*sigma;
			double nu = r-q-sigma2/2;
			double pd = -(sigma2/dx-nu)/(2*dx);
			double pu = -(sigma2/dx+nu)/(2*dx);
			double pm = sigma2/(dx*dx)+r;
			setMidRows(pd,pm,pu);
		}
	
	}

}
