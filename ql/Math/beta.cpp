
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

/*! \file beta.cpp
    \brief Beta and beta incomplete functions
*/

#include <ql/Math/beta.hpp>

namespace QuantLib {

    /*
        The implementation of the algorithm was inspired by
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    double betaContinuedFraction(double a, double  b, double  x,
        double accuracy, int maxIteration) {

    	double aa, del;
    	double qab=a+b;
    	double qap=a+1.0;
    	double qam=a-1.0;
    	double c=1.0;
    	double d=1.0-qab*x/qap;
    	if (QL_FABS(d) < QL_EPSILON) d=QL_EPSILON;
    	d=1.0/d;
    	double result=d;

    	int m, m2;
    	for (m=1; m<=maxIteration; m++) {
    		m2=2*m;
    		aa=m*(b-m)*x/((qam+m2)*(a+m2));
    		d=1.0+aa*d;
    		if (fabs(d) < QL_EPSILON) d=QL_EPSILON;
    		c=1.0+aa/c;
    		if (fabs(c) < QL_EPSILON) c=QL_EPSILON;
    		d=1.0/d;
    		result *= d*c;
    		aa = -(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
    		d=1.0+aa*d;
    		if (fabs(d) < QL_EPSILON) d=QL_EPSILON;
    		c=1.0+aa/c;
    		if (fabs(c) < QL_EPSILON) c=QL_EPSILON;
    		d=1.0/d;
    		del=d*c;
    		result *= del;
    		if (QL_FABS(del-1.0) < accuracy)
            	return result;
    	}
    	QL_FAIL("betaContinuedFraction : "
                "a or b too big, or maxIteration too small in betacf");
    }


    double incompleteBetaFunction(double a, double b, 
                                  double x, double accuracy,
                                  int maxIteration) {

    	QL_REQUIRE(a > 0.0,
    	    "betaIncompleteFunction : "
    	    "a must be greater than zero");

    	QL_REQUIRE(b > 0.0,
    	    "betaIncompleteFunction : "
    	    "b must be greater than zero");


    	if (x == 0.0)
    	    return 0.0;
    	else if (x == 1.0)
    	    return 1.0;
    	else
    	    QL_REQUIRE(x>0.0 && x<1.0,
                "betaIncompleteFunction : "
    	        "x must be in [0,1]");

    	double result = QL_EXP(GammaFunction().logValue(a+b) -
    		GammaFunction().logValue(a) - GammaFunction().logValue(b) +
    		a*QL_LOG(x) + b*QL_LOG(1.0-x));

    	if (x < (a+1.0)/(a+b+2.0))
    		return result *
    		    betaContinuedFraction(a, b, x, accuracy, maxIteration)/a;
    	else
    		return 1.0 - result *
    		    betaContinuedFraction(b, a, 1.0-x, accuracy, maxIteration)/b;
    }

}
