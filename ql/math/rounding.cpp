/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

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

    static inline Real fast_pow10(Integer precision) {
        // Providing support for truncating up to 16 decimal places after dot
        constexpr static double pow10_lut[0x20] = {
            1.0E0,  1.0E1,  1.0E2,  1.0E3,  1.0E4,  1.0E5,
            1.0E6,  1.0E7,  1.0E8,  1.0E9,  1.0E10, 1.0E11,
            1.0E12, 1.0E13, 1.0E14, 1.0E15, 1.0E16
            /*the rest of the numbers are zeros*/};
        // Skipping precision input value checks without causing a crash
        return pow10_lut[precision & 0x1F];
    }

    Decimal Rounding::operator()(Decimal value) const {

        if (type_ == None)
            return value;

        Real mult = fast_pow10(precision_);
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
