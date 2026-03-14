/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

#include <ql/math/beta.hpp>

namespace QuantLib {

    /*
      The implementation of the algorithm was inspired by
      "Numerical Recipes in C", 2nd edition,
      Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    Real betaContinuedFraction(Real a, Real  b, Real  x,
                               Real accuracy, Integer maxIteration) {

        Real aa, del;
        Real qab = a+b;
        Real qap = a+1.0;
        Real qam = a-1.0;
        Real c = 1.0;
        Real d = 1.0-qab*x/qap;
        if (std::fabs(d) < QL_EPSILON)
            d = QL_EPSILON;
        d = 1.0/d;
        Real result = d;

        Integer m, m2;
        for (m=1; m<=maxIteration; m++) {
            m2=2*m;
            aa=m*(b-m)*x/((qam+m2)*(a+m2));
            d=1.0+aa*d;
            if (std::fabs(d) < QL_EPSILON) d=QL_EPSILON;
            c=1.0+aa/c;
            if (std::fabs(c) < QL_EPSILON) c=QL_EPSILON;
            d=1.0/d;
            result *= d*c;
            aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
            d=1.0+aa*d;
            if (std::fabs(d) < QL_EPSILON) d=QL_EPSILON;
            c=1.0+aa/c;
            if (std::fabs(c) < QL_EPSILON) c=QL_EPSILON;
            d=1.0/d;
            del=d*c;
            result *= del;
            if (std::fabs(del-1.0) < accuracy)
                return result;
        }
        QL_FAIL("a or b too big, or maxIteration too small in betacf");
    }

    Real incompleteBetaFunction(Real a, Real b,
                                Real x, Real accuracy,
                                Integer maxIteration) {

    QL_REQUIRE(a > 0.0, "a must be greater than zero");
    QL_REQUIRE(b > 0.0, "b must be greater than zero");


    if (x == 0.0)
        return 0.0;
    else if (x == 1.0)
        return 1.0;
    else
        QL_REQUIRE(x>0.0 && x<1.0, "x must be in [0,1]");

    Real result = std::exp(GammaFunction().logValue(a+b) -
        GammaFunction().logValue(a) - GammaFunction().logValue(b) +
        a*std::log(x) + b*std::log(1.0-x));

    if (x < (a+1.0)/(a+b+2.0))
        return result *
            betaContinuedFraction(a, b, x, accuracy, maxIteration)/a;
    else
        return 1.0 - result *
            betaContinuedFraction(b, a, 1.0-x, accuracy, maxIteration)/b;
    }

}
