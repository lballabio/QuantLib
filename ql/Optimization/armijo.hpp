
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file armijo.hpp
    \brief Armijo line-search class

    \fullpath
    ql/Optimization/%armijo.hpp
*/

#ifndef quantlib_optimization_armijo_h
#define quantlib_optimization_armijo_h

#include "ql/Optimization/linesearch.hpp"

/*!
  Armijo linesearch.

  Let alpha and beta be 2 scalars in [0,1].
  Let x be the current value of the unknow, d the search direction and
  t the step. Let f be the function to minimize.
  The line search stop when t verifies
  f(x+t*d) - f(x) <= -alpha*t*f'(x+t*d) and 
  f(x+t/beta*d) - f(x) > -alpha*t*f'(x+t*d)/beta

  (see Polak. Algorithms and consitent approximations, Optimization,
  volume 124 of Apllied Mathematical Sciences. Springer-Arrayerlag, N-Y, 1997)
*/

namespace QuantLib {

    namespace Optimization {

        class ArmijoLineSearch : public LineSearch {
          public:
            //! Default constructor
            ArmijoLineSearch (double eps = 1e-8, 
                              double alpha = 0.5,
                              double beta = 0.65)
            : LineSearch(eps), alpha_(alpha), beta_(beta) {}
            //! Destructor
            virtual ~ ArmijoLineSearch () {}

            //! Perform line search
            virtual double operator () (
                OptimizationProblem &P,// Optimization problem
                double t_ini,      // initial value of line-search step
                double q0,         // function value
                double qp0);       // squared norm of gradient vector
          private:
            //! Armijo paramters
            double alpha_, beta_;
        };

    }

}


#endif
