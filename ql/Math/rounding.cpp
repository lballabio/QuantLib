
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

#include <ql/Math/rounding.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Decimal Rounding::round(const Decimal value) const {
        if (type_ == None) 
            return value;
        Decimal mult = pow(10.0,precision_);
        bool neg = (value < 0.0);
        Decimal lvalue = QL_FABS(value)*mult;
        Decimal integral = 0.0;
        Decimal modVal = QL_MODF(lvalue,&integral);
        switch (type_) {
          case Up:
            lvalue -= modVal;
            if (modVal >= (digit_/10.0))
                lvalue += 1.0;
            break;
          case Down:
            lvalue -= modVal;
            break;
          case Floor:
            lvalue -= modVal;
            if (!neg) {
                if (modVal >= (digit_/10.0))
                lvalue += 1.0;
            }
            break;
          case Ceiling:
            lvalue -= modVal;
            if (neg) {
                if (modVal >= (digit_/10.0))
                lvalue += 1.0;
            }
            break;
          default:
            QL_FAIL("unknown rounding method");
        }
        return (neg) ? (lvalue/mult)*-1.0 : lvalue/mult;
    }

}

