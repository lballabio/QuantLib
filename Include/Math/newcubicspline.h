		
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

#ifndef ql_cubic_spline_h
#define ql_cubic_spline_h

#include "array.h"
#include "location.h"


namespace QuantLib{

	class NewCubicSpline{
	  public:
		// constructor
		  NewCubicSpline(Array xvalues, Array yvalues, double dy1=1.0e+30, double dyn=1.0e+30);
		// accessors
//		double value(double x, int guess = -1) const;
//		double firstDerivative(double x, int guess = -1) const;
		double value(double x, int guess ) const;
		double firstDerivative(double x, int guess) const;
		private:
		Array xValues, yValues, coeffs;
		bool isStrictlyAscending(Array v);
	};

}
#endif
