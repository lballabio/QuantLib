
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

/*! \file conjugategradient.hpp
    \brief Conjugate gradient optimization method

    \fullpath
    ql/Optimization/%conjugategradient.hpp
*/

#ifndef quantlib_optimization_conjugate_gradient_h
#define quantlib_optimization_conjugate_gradient_h

#include <iostream>
#include <iomanip>
#include <cmath>

#include "ql/array.hpp"
#include "ql/handle.hpp"

#include "ql/Optimization/optimizer.hpp"
#include "ql/Optimization/linesearch.hpp"
#include "ql/Optimization/criteria.hpp"
#include "ql/Optimization/armijo.hpp"


/*!
  Multi-dimensionnal Conjugate Gradient class
  User has to provide line-search method and
  optimization end criteria
  
  search direction d_i = - f'(x_i) + c_i*d_{i-1}
  where c_i = ||f'(x_i)||^2/||f'(x_{i-1})||^2 
  and d_1 = - f'(x_1)
  
*/
namespace QuantLib {

    namespace Optimization {

        class ConjugateGradient: public OptimizationMethod {
            //! line search
            Handle<LineSearch> lineSearch_;
          public:
            //! default constructor
            ConjugateGradient() : OptimizationMethod(),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch() )) {}
            //! default constructor
            ConjugateGradient(Handle<LineSearch>& lineSearch)    // Reference to a line search method
            : OptimizationMethod(), lineSearch_ (lineSearch) {}


            //! destructor
            virtual ~ConjugateGradient() {}

            //! minimize the optimization problem P
            virtual void Minimize(OptimizationProblem& P);
        };

    }

}


#endif
