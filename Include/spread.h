
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
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
