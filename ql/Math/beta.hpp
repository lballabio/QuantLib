
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

/*! \file beta.hpp
    \brief Beta and beta incomplete functions
*/

#ifndef quantlib_math_beta_h
#define quantlib_math_beta_h

#include <ql/Math/gammadistribution.hpp>

namespace QuantLib {

    inline double betaFunction(double z,
                        double w) {
    	return QL_EXP(GammaFunction().logValue(z) +
                      GammaFunction().logValue(w) -
                      GammaFunction().logValue(z+w));
    }

    double betaContinuedFraction(double a,
                                 double b,
                                 double x,
                                 double accuracy = 1e-16,
                                 int maxIteration = 100);

    //! Incomplete Beta function
    /*! Incomplete Beta function

        The implementation of the algorithm was inspired by
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    double incompleteBetaFunction(double a,
                                  double b,
                                  double x,
                                  double accuracy = 1e-16,
                                  int maxIteration = 100);

}
#endif
