
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_yield_h
#define quantlib_yield_h

#include "qldefines.h"
#include "qlerrors.h"
#include "spread.h"
#include "dataformatters.h"

namespace QuantLib {

	#ifdef QL_DEBUG
	
		class Rate {
			// arithmetic operators
			friend Rate operator+(Rate y1, Rate y2) { return Rate(y1.yield+y2.yield); }
			friend Rate operator+(Rate y, Spread s) { return Rate(y.yield+double(s)); }
			friend Rate operator+(Spread s, Rate y) { return Rate(double(s)+y.yield); }
			friend double operator+(Rate y, double x) { return y.yield+x; }
			friend double operator+(double x, Rate y) { return x+y.yield; }
			friend Spread operator-(Rate y1, Rate y2) { return Spread(y1.yield-y2.yield); }
			friend Rate operator-(Rate y, Spread s) { return Rate(y.yield-double(s)); }
			friend double operator-(Rate y, double x) { return y.yield-x; }
			friend double operator-(double x, Rate y) { return x-y.yield; }
			friend double operator*(Rate y, double x) { return y.yield*x; }
			friend double operator*(double x, Rate y) { return y.yield*x; }
			friend Rate operator/(Rate y, double x) { return Rate(y.yield/x); }
			// comparison operators
			friend bool operator==(Rate y1, Rate y2) { return (y1.yield==y2.yield); }
			friend bool operator!=(Rate y1, Rate y2) { return (y1.yield!=y2.yield); }
			friend bool operator<(Rate y1, Rate y2) { return (y1.yield<y2.yield); }
			friend bool operator<=(Rate y1, Rate y2) { return (y1.yield<=y2.yield); }
			friend bool operator>(Rate y1, Rate y2) { return (y1.yield>y2.yield); }
			friend bool operator>=(Rate y1, Rate y2) { return (y1.yield>=y2.yield); }
		  public:
			// constructor
			Rate(double y = 0.0);
			// assignment
			Rate& operator=(double y);
			Rate& operator=(const Rate&);
			// cast
			operator double() const { return yield; }
			// computed assignment
			Rate& operator+=(Rate y) { return (*this = Rate(yield+y.yield)); }
			Rate& operator+=(Spread s) { return (*this = Rate(yield+double(s))); }
			Rate& operator-=(Rate y) { return (*this = Rate(yield-y.yield)); }
			Rate& operator-=(Spread s) { return (*this = Rate(yield-double(s))); }
			Rate& operator*=(double x) { return (*this = Rate(yield*x)); }
			Rate& operator/=(double x) { return (*this = Rate(yield/x)); }
		  private:
			double yield;
		};
	
	
		// inline definitions
	
		inline Rate::Rate(double y) {
			Require(y >= 0.0 && y <= 1.0,
				"invalid yield value ("+RateFormat(y)+"). Value must be between 0 and 1");
			yield = y;
		}
	
		inline Rate& Rate::operator=(double y) {
			Require(y >= 0.0 && y <= 1.0,
				"invalid yield value ("+RateFormat(y)+"). Value must be between 0 and 1");
			yield = y;
			return *this;
		}
	
		inline Rate& Rate::operator=(const Rate& y) {
			yield = y.yield;
			return *this;
		}
	
	#else
	
		typedef double Rate;
	
	#endif

}


#endif

