
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/Math/factorial.hpp>
#include <ql/Math/gammadistribution.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        const double firstFactorials[] = {
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

        const unsigned int tabulated = 
            sizeof(firstFactorials)/sizeof(firstFactorials[0])-1;

    }

    double Factorial::get(unsigned int i) {
        if (i<=tabulated) {
            return firstFactorials[i];
        } else {
            return QL_EXP(GammaFunction().logValue(i+1));
        }
    }

    double Factorial::ln(unsigned int i) {
        if (i<=tabulated) {
            return QL_LOG(firstFactorials[i]);
        } else {
            return GammaFunction().logValue(i+1);
        }
    }

}
