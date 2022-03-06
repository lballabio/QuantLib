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

/*! \file beta.hpp
    \brief Beta and beta incomplete functions
*/

#ifndef quantlib_math_beta_h
#define quantlib_math_beta_h

#include <ql/math/distributions/gammadistribution.hpp>

namespace QuantLib {

    inline Real betaFunction(Real z, Real w) {
    return std::exp(GammaFunction().logValue(z) +
                    GammaFunction().logValue(w) -
                    GammaFunction().logValue(z+w));
    }

    Real betaContinuedFraction(Real a,
                               Real b,
                               Real x,
                               Real accuracy = 1e-16,
                               Integer maxIteration = 100);

    //! Incomplete Beta function
    /*! Incomplete Beta function

        The implementation of the algorithm was inspired by
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    Real incompleteBetaFunction(Real a,
                                Real b,
                                Real x,
                                Real accuracy = 1e-16,
                                Integer maxIteration = 100);

}


#endif


#ifndef id_90754e8290805c021f662d9eb76e42a4
#define id_90754e8290805c021f662d9eb76e42a4
inline bool test_90754e8290805c021f662d9eb76e42a4(const int* i) {
    return i != nullptr;
}
#endif
