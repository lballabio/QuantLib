
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

/*! \file costfunction.hpp
    \brief Optimization cost function class

    \fullpath
    ql/Optimization/%costfunction.hpp
*/

#ifndef quantlib_optimization_costfunction_h
#define quantlib_optimization_costfunction_h

/*! 
  Cost function abstract class for unconstrained optimization pb
*/

namespace QuantLib {

    namespace Optimization {

        class CostFunction {
          public:
            //! method to overload to compute the cost functon value in x
            virtual double value(const Array& x) = 0;

            //! method to overload to compute grad_f, the first derivative of 
            //  the cost function with respect to x
            virtual void gradient(Array& grad_f, const Array& x) {
                double eps = finiteDifferenceEpsilon(), fp, fm;
                Array xx = x;
                int i, sz = x.size();
                for (i = 0; i < sz; ++i) {
                      xx[i] += eps;
                      fp = value(xx);
                      xx[i] -= 2. * eps;
                      fm = value(xx);
                      grad_f[i] = 0.5 * (fp - fm) / eps;
                }
            }

            //! method to overload to compute grad_f, the first derivative of 
            //  the cost function with respect to x and also the cost function
            virtual double valueAndGradient(Array& grad_f, const Array& x) {
                gradient(grad_f, x);
                return value(x);
            }

            //! Default epsilon for finite difference method : 
            virtual double finiteDifferenceEpsilon() { return 1e-8; }
        };

    }

}

#endif
