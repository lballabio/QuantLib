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

/*
    The implementation of the algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 6
*/

#include <ql/math/incompletegamma.hpp>
#include <ql/math/distributions/gammadistribution.hpp>

namespace QuantLib {


    Real incompleteGammaFunction(Real a, Real x, Real accuracy,
                                 Integer maxIteration) {

        QL_REQUIRE(a>0.0, "non-positive a is not allowed");

        QL_REQUIRE(x>=0.0, "negative x non allowed");

        if (x < (a+1.0)) {
            // Use the series representation
            return incompleteGammaFunctionSeriesRepr(a, x,
                accuracy, maxIteration);
        } else {
            // Use the continued fraction representation
            return 1.0-incompleteGammaFunctionContinuedFractionRepr(a, x,
                accuracy, maxIteration);
        }

    }


    Real incompleteGammaFunctionSeriesRepr(Real a, Real x, Real accuracy,
                                           Integer maxIteration) {

        if (x==0.0) return 0.0;

        Real gln = GammaFunction().logValue(a);
        Real ap=a;
        Real del=1.0/a;
        Real sum=del;
        for (Integer n=1; n<=maxIteration; n++) {
            ++ap;
            del *= x/ap;
            sum += del;
            if (std::fabs(del) < std::fabs(sum)*accuracy) {
                return sum*std::exp(-x+a*std::log(x)-gln);
            }
        }
        QL_FAIL("accuracy not reached");
    }

    Real incompleteGammaFunctionContinuedFractionRepr(Real a, Real x,
                                                      Real accuracy,
                                                      Integer maxIteration) {

        Integer i;
        Real an, b, c, d, del, h;
        Real gln = GammaFunction().logValue(a);
        b=x+1.0-a;
        c=1.0/QL_EPSILON;
        d=1.0/b;
        h=d;
        for (i=1; i<=maxIteration; i++) {
            an = -i*(i-a);
            b += 2.0;
            d=an*d+b;
            if (std::fabs(d) < QL_EPSILON) d=QL_EPSILON;
            c=b+an/c;
            if (std::fabs(c) < QL_EPSILON) c=QL_EPSILON;
            d=1.0/d;
            del=d*c;
            h *= del;
            if (std::fabs(del-1.0) < accuracy) {
                return std::exp(-x+a*std::log(x)-gln)*h;
            }
        }

        QL_FAIL("accuracy not reached");
    }



}
