
/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

/*! \file armijo.hpp
    \brief Armijo line-search class
*/

#ifndef quantlib_optimization_armijo_h
#define quantlib_optimization_armijo_h

#include <ql/Optimization/linesearch.hpp>

namespace QuantLib {

    //! Armijo line search.
    /*! Let alpha and beta be 2 scalars in [0,1].
        Let x be the current value of the unknow, d the search direction and
        t the step. Let f be the function to minimize.
        The line search stop when t verifies
        f(x+t*d) - f(x) <= -alpha*t*f'(x+t*d) and
        f(x+t/beta*d) - f(x) > -alpha*t*f'(x+t*d)/beta

        (see Polak. Algorithms and consitent approximations, Optimization,
        volume 124 of Applied Mathematical Sciences. Springer-Arrayerlag, N-Y,
        1997)
    */
    class ArmijoLineSearch : public LineSearch {
      public:
        //! Default constructor
        ArmijoLineSearch(Real eps = 1e-8,
                         Real alpha = 0.5,
                         Real beta = 0.65)
        : LineSearch(eps), alpha_(alpha), beta_(beta) {}
        //! Destructor
        virtual ~ArmijoLineSearch () {}

        //! Perform line search
        virtual Real operator() (
                       const Problem& P,  // Optimization problem
                       Real t_ini);     // initial value of line-search step
      private:
        //! Armijo paramters
        Real alpha_, beta_;
    };

}


#endif
