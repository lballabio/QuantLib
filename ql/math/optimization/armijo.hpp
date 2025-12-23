/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

/*! \file armijo.hpp
    \brief Armijo line-search class
*/

#ifndef quantlib_optimization_armijo_hpp
#define quantlib_optimization_armijo_hpp

#include <ql/math/optimization/linesearch.hpp>

namespace QuantLib {

    class EndCriteria;

    //! Armijo line search.
    /*! Let \f$ \alpha \f$ and \f$ \beta \f$ be 2 scalars in \f$ [0,1]
        \f$.  Let \f$ x \f$ be the current value of the unknown, \f$ d
        \f$ the search direction and \f$ t \f$ the step. Let \f$ f \f$
        be the function to minimize.  The line search stops when \f$ t
        \f$ verifies
        \f[ f(x + t \cdot d) - f(x) \leq -\alpha t f'(x+t \cdot d) \f]
        and
        \f[ f(x+\frac{t}{\beta} \cdot d) - f(x) > -\frac{\alpha}{\beta}
            t f'(x+t \cdot d) \f]

        (see Polak, Algorithms and consistent approximations, Optimization,
        volume 124 of Applied Mathematical Sciences, Springer-Verlag, NY,
        1997)
    */
    class ArmijoLineSearch : public LineSearch {
      public:
        //! Default constructor
        ArmijoLineSearch(Real eps = 1e-8,
                         Real alpha = 0.05,
                         Real beta = 0.65)
        : LineSearch(eps), alpha_(alpha), beta_(beta) {}

        //! Perform line search
        Real operator()(Problem& P, // Optimization problem
                        EndCriteria::Type& ecType,
                        const EndCriteria&,
                        Real t_ini) override; // initial value of line-search step
      private:
        Real alpha_, beta_;
    };

}

#endif
