
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

#ifndef quantlib_discount_factor_h
#define quantlib_discount_factor_h

#include "qldefines.h"
#include "qlerrors.h"
#include "dataformatters.h"

namespace QuantLib {

	#ifdef QL_DEBUG
	
		class DiscountFactor {
		  public:
			friend DiscountFactor operator*(DiscountFactor x, DiscountFactor y) { return DiscountFactor(x.factor*y.factor); }
			friend double operator*(DiscountFactor x, double y) { return x.factor*y; }
			friend double operator*(double x, DiscountFactor y) { return x*y.factor; }
			friend double operator/(DiscountFactor x, DiscountFactor y) { return x.factor/y.factor; }
			friend double operator/(DiscountFactor x, double y) { return x.factor/y; }
			friend double operator/(double x, DiscountFactor y) { return x/y.factor; }
			// constructor
			DiscountFactor(double f = 0.0);
			// assignment
			DiscountFactor& operator=(double);
			// cast
			operator double() const { return factor; }
			// computed assignment
			DiscountFactor& operator*=(DiscountFactor x) { return (*this = DiscountFactor(factor*x.factor)); }
		  private:
			double factor;
		};
	
		// inline definitions
	
		inline DiscountFactor::DiscountFactor(double f) {
			Require(f >= 0.0 && f <= 1.0,"invalid discount factor ("+DoubleFormatter::toString(f)+"). Value must be between 0 and 1");
			factor = f;
		}
	
		inline DiscountFactor& DiscountFactor::operator=(double f) {
			Require(f >= 0.0 && f <= 1.0,"invalid discount factor ("+DoubleFormatter::toString(f)+"). Value must be between 0 and 1");
			factor = f;
			return *this;
		}
	
	#else
	
		//! used to describe discount factors between dates.
		typedef double DiscountFactor;
	
	#endif

}


#endif

