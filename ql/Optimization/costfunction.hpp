/*
 Copyright (C) 2001, 2002 Nicolas Di Césaré

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file costfunction.hpp
    \brief Optimization cost function class

    \fullpath
    ql/Optimization/%costfunction.hpp
*/

#ifndef quantlib_optimization_costfunction_h
#define quantlib_optimization_costfunction_h

#include <ql/qldefines.hpp>
#include <ql/array.hpp>

namespace QuantLib {

    namespace Optimization {

        //!  Cost function abstract class for optimization problem
        class CostFunction {
          public:
            //! method to overload to compute the cost functon value in x
            virtual double value(const Array& x) = 0;

            //! method to overload to compute grad_f, the first derivative of
            //  the cost function with respect to x
            virtual void gradient(Array& grad, const Array& x) {
                double eps = finiteDifferenceEpsilon(), fp, fm;
                Array xx(x);

                for (Size i=0; i<x.size(); i++) {
                    xx[i] += eps;
                    fp = value(xx);
                    xx[i] -= 2.0*eps;
                    fm = value(xx);
                    grad[i] = 0.5*(fp - fm)/eps;
                    xx[i] = x[i];
                }
                std::cout << "Gradient at " << x << " = " << grad << std::endl;
            }

            //! method to overload to compute grad_f, the first derivative of
            //  the cost function with respect to x and also the cost function
            virtual double valueAndGradient(Array& grad, const Array& x) {
                gradient(grad, x);
                return value(x);
            }

            //! Default epsilon for finite difference method :
            virtual double finiteDifferenceEpsilon() { return 1e-8; }
        };

    }

}

#endif
