
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

/*
    The implementation of the algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 6
*/

#include <ql/Math/incompletegamma.hpp>
#include <ql/Math/gammadistribution.hpp>

namespace QuantLib {


    double incompleteGammaFunction(double a, double x, double accuracy,
        int maxIteration) {

        QL_REQUIRE(a>0.0,
            "incompleteGammaFunction :"
            "non-positive a is not allowed");

        QL_REQUIRE(x>=0.0,
            "incompleteGammaFunction :"
            "negative x non allowed");

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


    double incompleteGammaFunctionSeriesRepr(double a, double x, double accuracy,
        int maxIteration) {

        if (x==0.0) return 0.0;

        double gln = GammaFunction().logValue(a);
        double ap=a;
        double del=1.0/a;
        double sum=del;
        for (int n=1; n<=maxIteration; n++) {
            ++ap;
            del *= x/ap;
            sum += del;
            if (QL_FABS(del) < QL_FABS(sum)*accuracy) {
                return sum*QL_EXP(-x+a*QL_LOG(x)-gln);
            }
        }
        QL_FAIL("incompleteGammaFunctionSeriesRepr : "
                "accuracy not reached with maxIteration.");
    }

    double incompleteGammaFunctionContinuedFractionRepr(double a, double x,
        double accuracy, int maxIteration) {

        int i;
        double an, b, c, d, del, h;
        double gln = GammaFunction().logValue(a);
        b=x+1.0-a;
        c=1.0/QL_EPSILON;
        d=1.0/b;
        h=d;
        for (i=1; i<=maxIteration; i++) {
            an = -i*(i-a);
            b += 2.0;
            d=an*d+b;
            if (QL_FABS(d) < QL_EPSILON) d=QL_EPSILON;
            c=b+an/c;
            if (QL_FABS(c) < QL_EPSILON) c=QL_EPSILON;
            d=1.0/d;
            del=d*c;
            h *= del;
            if (QL_FABS(del-1.0) < accuracy) {
                return QL_EXP(-x+a*QL_LOG(x)-gln)*h;
            }
        }

        QL_FAIL("incompleteGammaFunctionContinuedFractionRepr : "
                "accuracy not reached with maxIteration.");
    }



}
