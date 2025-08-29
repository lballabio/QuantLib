/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Jose Aparicio

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

/*! \file factorreduction.hpp
    \brief Single factor correlation reduction
*/

#ifndef quantlib_factorreduction_hpp
#define quantlib_factorreduction_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    /*! Iterative procedure to compute a correlation matrix reduction to
        a single factor dependence vector by minimizing the residuals.

        It assumes that such a reduction is possible, notice that if the
        dependence can not be reduced to one factor the correlation
        factors might be above 1.

        The matrix passed is destroyed.

        See for instance: "Modern Factor Analysis", Harry H. Harman,
          University Of Chicago Press, 1976. Chapter 9 is relevant to
          this context.
    */
    std::vector<Real> factorReduction(Matrix mtrx, Size maxIters = 25);

}

#endif
