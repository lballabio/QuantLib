
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
#include "formats.h"

QL_BEGIN_NAMESPACE(QuantLib)

#ifdef QL_DEBUG

	class Yield {
		// arithmetic operators
		friend Yield operator+(Yield y1, Yield y2) { return Yield(y1.yield+y2.yield); }
		friend Yield operator+(Yield y, Spread s) { return Yield(y.yield+double(s)); }
		friend Yield operator+(Spread s, Yield y) { return Yield(double(s)+y.yield); }
		friend double operator+(Yield y, double x) { return y.yield+x; }
		friend double operator+(double x, Yield y) { return x+y.yield; }
		friend Spread operator-(Yield y1, Yield y2) { return Spread(y1.yield-y2.yield); }
		friend Yield operator-(Yield y, Spread s) { return Yield(y.yield-double(s)); }
		friend double operator-(Yield y, double x) { return y.yield-x; }
		friend double operator-(double x, Yield y) { return x-y.yield; }
		friend double operator*(Yield y, double x) { return y.yield*x; }
		friend double operator*(double x, Yield y) { return y.yield*x; }
		friend Yield operator/(Yield y, double x) { return Yield(y.yield/x); }
		// comparison operators
		friend bool operator==(Yield y1, Yield y2) { return (y1.yield==y2.yield); }
		friend bool operator!=(Yield y1, Yield y2) { return (y1.yield!=y2.yield); }
		friend bool operator<(Yield y1, Yield y2) { return (y1.yield<y2.yield); }
		friend bool operator<=(Yield y1, Yield y2) { return (y1.yield<=y2.yield); }
		friend bool operator>(Yield y1, Yield y2) { return (y1.yield>y2.yield); }
		friend bool operator>=(Yield y1, Yield y2) { return (y1.yield>=y2.yield); }
	  public:
		// constructor
		Yield(double y = 0.0);
		// assignment
		Yield& operator=(double y);
		Yield& operator=(const Yield&);
		// cast
		operator double() const { return yield; }
		// computed assignment
		Yield& operator+=(Yield y) { return (*this = Yield(yield+y.yield)); }
		Yield& operator+=(Spread s) { return (*this = Yield(yield+double(s))); }
		Yield& operator-=(Yield y) { return (*this = Yield(yield-y.yield)); }
		Yield& operator-=(Spread s) { return (*this = Yield(yield-double(s))); }
		Yield& operator*=(double x) { return (*this = Yield(yield*x)); }
		Yield& operator/=(double x) { return (*this = Yield(yield/x)); }
	  private:
		double yield;
	};


	// inline definitions

	inline Yield::Yield(double y) {
		Require(y >= 0.0 && y <= 1.0,
			"invalid yield value ("+RateFormat(y)+"). Value must be between 0 and 1");
		yield = y;
	}

	inline Yield& Yield::operator=(double y) {
		Require(y >= 0.0 && y <= 1.0,
			"invalid yield value ("+RateFormat(y)+"). Value must be between 0 and 1");
		yield = y;
		return *this;
	}

	inline Yield& Yield::operator=(const Yield& y) {
		yield = y.yield;
		return *this;
	}

#else

	typedef double Yield;

#endif

QL_END_NAMESPACE(QuantLib)


#endif

