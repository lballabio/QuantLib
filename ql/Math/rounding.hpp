
/*
 Copyright (C) 2004 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rounding.hpp
    \brief Rounding implementation
*/

#ifndef quantlib_rounding_hpp
#define quantlib_rounding_hpp

#include <ql/types.hpp>

namespace QuantLib {

    //! Type of rounding
    
    enum RoundingType {
	ROUND_UP,
	ROUND_DOWN,
	ROUND_FLOOR,
	ROUND_CEILING,
	DONT_ROUND
    };
    
    //! Basic rounding.
    /*! Round up means first decimal place past precision will be rounded up
      if greater than digit.
      Round down means all decimal places past precision will be truncated.
    */

    class Rounding {
    public:
	Rounding() {}
	Rounding(const long precision,
		 const RoundingType type = ROUND_UP,
		 const int digit = 5)
	: precision_(precision),type_(type),digit_(digit) {}
	Decimal round(const Decimal value) const;
    private:
	long precision_;
	RoundingType type_;
	int digit_;
    };

    //! Ceiling truncation.
    /*! Positive numbers will be rounded down, negative numbers will be
      rounded up.
    */
    
    class CeilingTruncation : public Rounding {
	CeilingTruncation(const long precision, const int digit = 5)
	: Rounding(precision,ROUND_CEILING,digit) {}
    };
    
    //! Floor truncation.
    /*! Positive numbers will be rounded up, negative numbers will be
      rounded down.
    */
    
    class FloorTruncation : public Rounding {
	FloorTruncation(const long precision, const int digit = 5)
	: Rounding(precision,ROUND_FLOOR,digit) {}
    };
    
    inline Decimal Rounding::round(const Decimal value) const {
	if (type_ == DONT_ROUND) return value;
	double mult = pow(10.0,precision_);
	bool neg = (value < 0.0);
	double lvalue = QL_FABS(value)*mult;
	double integral = 0.0;
	double modVal = QL_MODF(lvalue,&integral);
	switch (type_) {
	    case ROUND_UP:
		lvalue -= modVal;
		if (modVal >= (digit_/10.0))
		    lvalue += 1.0;
		break;
	    case ROUND_DOWN:
		lvalue -= modVal;
		break;
	    case ROUND_FLOOR:
		lvalue -= modVal;
		if (!neg) {
		    if (modVal >= (digit_/10.0))
			lvalue += 1.0;
		}
		break;
	    case ROUND_CEILING:
		lvalue -= modVal;
		if (neg) {
		    if (modVal >= (digit_/10.0))
			lvalue += 1.0;
		}
		break;
	    default:
		break;
	}
	return (neg) ? (lvalue/mult)*-1.0 : lvalue/mult;
    }
    
}

#endif
