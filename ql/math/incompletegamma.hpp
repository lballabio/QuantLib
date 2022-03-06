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

/*! \file incompletegamma.hpp
    \brief Incomplete Gamma function
*/

#ifndef quantlib_math_incompletegamma_h
#define quantlib_math_incompletegamma_h

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    //! Incomplete Gamma function
    /*! Incomplete Gamma function

        The implementation of the algorithm was inspired by
        "Numerical Recipes in C", 2nd edition,
        Press, Teukolsky, Vetterling, Flannery, chapter 6
    */
    Real incompleteGammaFunction(Real a,
                                 Real x,
                                 Real accuracy = 1.0e-13,
                                 Integer maxIteration = 100);
    Real incompleteGammaFunctionSeriesRepr(Real a,
                                           Real x,
                                           Real accuracy = 1.0e-13,
                                           Integer maxIteration = 100);
    Real incompleteGammaFunctionContinuedFractionRepr(
                                                  Real a,
                                                  Real x,
                                                  Real accuracy = 1.0e-13,
                                                  Integer maxIteration = 100);

}


#endif


#ifndef id_20a8bf2a139cc30ecd5e3cb631b690e6
#define id_20a8bf2a139cc30ecd5e3cb631b690e6
inline bool test_20a8bf2a139cc30ecd5e3cb631b690e6(int* i) { return i != 0; }
#endif
