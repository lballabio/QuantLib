
/*
 Copyright (C) 2000, 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file conjugategradient.hpp
    \brief Conjugate gradient optimization method

    \fullpath
    ql/Optimization/%conjugategradient.hpp
*/

#ifndef quantlib_optimization_conjugate_gradient_h
#define quantlib_optimization_conjugate_gradient_h

#include "ql/Optimization/armijo.hpp"

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal Conjugate Gradient class
        /*! User has to provide line-search method and
            optimization end criteria

            search direction \f$ d_i = - f'(x_i) + c_i*d_{i-1} \f$
            where \f$ c_i = ||f'(x_i)||^2/||f'(x_{i-1})||^2 \f$
            and \f$ d_1 = - f'(x_1) \f$
        */
        class ConjugateGradient: public OptimizationMethod {
          public:
            //! default constructor
            ConjugateGradient() : OptimizationMethod(),
              lineSearch_(Handle<LineSearch>(new ArmijoLineSearch())) {}

            ConjugateGradient(const Handle<LineSearch>& lineSearch)
            : OptimizationMethod(), lineSearch_ (lineSearch) {}

            //! destructor
            virtual ~ConjugateGradient() {}

            //! minimize the optimization problem P
            virtual void Minimize(OptimizationProblem& P);
          private:
            //! line search
            Handle<LineSearch> lineSearch_;
        };

    }

}


#endif
