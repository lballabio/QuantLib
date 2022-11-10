/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rounding.hpp
    \brief Rounding implementation
*/

#include <ql/math/rounding.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Decimal Rounding::operator()(Decimal value) const {

        if (type_ == None)
            return value;

        Real mult = std::pow(10.0,precision_);
        bool neg = (value < 0.0);
        Real lvalue = std::fabs(value)*mult;
        Real integral = 0.0;
        Real modVal = std::modf(lvalue,&integral);
        lvalue -= modVal;
        switch (type_) {
          case Down:
            break;
          case Up:
            if (modVal != 0.0)
                lvalue += 1.0;
            break;
          case Closest:
            if (modVal >= (digit_/10.0))
                lvalue += 1.0;
            break;
          case Floor:
            if (!neg) {
                if (modVal >= (digit_/10.0))
                    lvalue += 1.0;
            }
            break;
          case Ceiling:
            if (neg) {
                if (modVal >= (digit_/10.0))
                    lvalue += 1.0;
            }
            break;
          default:
            QL_FAIL("unknown rounding method");
        }
        return (neg) ? Real(-(lvalue / mult)) : Real(lvalue / mult);
    }

}
