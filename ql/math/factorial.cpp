/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/math/factorial.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        const Real firstFactorials[] = {
                                   1.0,                                   1.0,
                                   2.0,                                   6.0,
                                  24.0,                                 120.0,
                                 720.0,                                5040.0,
                               40320.0,                              362880.0,
                             3628800.0,                            39916800.0,
                           479001600.0,                          6227020800.0,
                         87178291200.0,                       1307674368000.0,
                      20922789888000.0,                     355687428096000.0,
                    6402373705728000.0,                  121645100408832000.0,
                 2432902008176640000.0,                51090942171709440000.0,
              1124000727777607680000.0,             25852016738884976640000.0,
            620448401733239439360000.0,          15511210043330985984000000.0,
         403291461126605635584000000.0,       10888869450418352160768000000.0
        };

        const Size tabulated =
            sizeof(firstFactorials)/sizeof(firstFactorials[0])-1;

    }

    Real Factorial::get(Natural i) {
        if (i<=tabulated) {
            return firstFactorials[i];
        } else {
            return std::exp(GammaFunction().logValue(i+1));
        }
    }

    Real Factorial::ln(Natural i) {
        if (i<=tabulated) {
            return std::log(firstFactorials[i]);
        } else {
            return GammaFunction().logValue(i+1);
        }
    }

}
