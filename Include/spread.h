
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

/*! \file spread.h
	\brief spread on interest rates

	$Source$
	$Name$
	$Log$
	Revision 1.6  2000/12/14 12:32:29  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_spread_h
#define quantlib_spread_h

#include "qldefines.h"
#include "qlerrors.h"
#include "dataformatters.h"
#include <sstream>

namespace QuantLib {

	#ifdef QL_DEBUG
	
		class Spread {
			friend Spread operator+(Spread s1, Spread s2) { return Spread(s1.spread+s2.spread); }
			friend double operator+(Spread s, double x) { return s.spread+x; }
			friend double operator+(double x, Spread s) { return x+s.spread; }
			friend Spread operator-(Spread s1, Spread s2) { return Spread(s1.spread-s2.spread); }
			friend double operator-(Spread s, double x) { return s.spread-x; }
			friend double operator-(double x, Spread s) { return x-s.spread; }
			friend double operator*(Spread s, double x) { return s.spread*x; }
			friend double operator*(double x, Spread s) { return s.spread*x; }
			friend double operator/(Spread s, double x) { return s.spread/x; }
		  public:
			// constructor
			Spread(double s = 0.0);
			// assignment
			Spread& operator=(double s);
			// cast
			operator double() const { return spread; }
			// computed assignment
			Spread& operator+=(Spread s) { return (*this = Spread(spread+s.spread)); }
			Spread& operator-=(Spread s) { return (*this = Spread(spread-s.spread)); }
			Spread& operator*=(double x) { return (*this = Spread(spread*x)); }
			Spread& operator/=(double x) { return (*this = Spread(spread/x)); }
		  private:
			double spread;
		};
	
		// inline definitions
	
		inline Spread::Spread(double s) {
			Require(s >= -1.0 && s <= 1.0,
				"invalid spread value ("+RateFormatter::toString(s)+"). Absolute value must be between 0 and 1");
			spread = s;
		}
	
		inline Spread& Spread::operator=(double s) {
			Require(s >= -1.0 && s <= 1.0,
				"invalid spread value ("+RateFormatter::toString(s)+"). Absolute value must be between 0 and 1");
			spread = s;
			return *this;
		}
	
	#else
	
		//! used to describe spreads on interest rates.
		typedef double Spread;
	
	#endif

}



#endif
