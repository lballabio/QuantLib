
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

/*! \file armijo.cpp
    \brief Armijo line-search class

    \fullpath
    ql/Optimization/%armijo.cpp
*/

#include "ql/Optimization/armijo.hpp"

namespace QuantLib {

    namespace Optimization {

        double ArmijoLineSearch::operator()(
            OptimizationProblem &P,	// Optimization problem
            value_type t_ini,	// initial value of line-search step
            value_type q0,	// function value
            value_type qp0)	// squared norm of gradient vector
        {
            bool maxIter = false;
            qt_ = q0;
            qpt_ = qp0;
            double qtold, t = t_ini;
// is it needed?
//          double qptnew;
            int loopNumber = 0;

            OptimizationMethod &method = P.optimisationMethod ();
            Array & x = method.x ();
            Array & d = method.searchDirection ();

            // Initialize gradient
            gradient_ = Array (x.size ());
            // Compute new point
            xtd_ = x + t * d;
            // Compute function value at the new point
            qt_ = P.value (xtd_);

            // Enter in the loop if the criterion is not satisfied
#ifdef DEBUG_ARMIJO


                std::cout << "qt_ - q0 = " << (qt_ -
                               q0) << ", - alpha_ * t * qpt_ = "
                << (-alpha_ * t * qpt_) << std::endl;
#endif

            if ((qt_ - q0) > -alpha_ * t * qpt_) {
                do {
                    loopNumber++;
                    // Decrease step
                    t *= beta_;
                    // Store old value of the function
                    qtold = qt_;
                    // New point value
                    xtd_ = x + t * d;
                    // Compute function value at the new point
                    qt_ = P.value (xtd_);
                    P.firstDerivative (gradient_, xtd_);
                    // and it squared norm
// is it needed?
//                  qptnew = DotProduct (gradient_, gradient_);
#ifdef DEBUG_ARMIJO
                    std::cout << loopNumber << ", qt_ - q0 = " << (qt_ -
                                           q0) <<
                    ", - alpha_ * t * qpt_ = " << (-alpha_ * t *
                                       qpt_) << std::endl;
                    std::cout << "qtold - q0 = " << (qtold -
                                     q0) <<
                    ", - alpha_ * t * qpt_ / beta_ = " << (-alpha_ * t *
                                           qpt_ /
                                           beta_) << std::
                    endl;
#endif
                    maxIter =
                    P.optimisationMethod ().endCriteria ().
                    checkIterationNumber (loopNumber);
                }		// Armijo criteria
                  while (
                     (((qt_ - q0) > (-alpha_ * t * qpt_))
                      || ((qtold - q0) <= (-alpha_ * t * qpt_ / beta_)))
                     && (!maxIter));
              }

            if (maxIter)
              {
                  succeed_ = false;
              }

            // Compute new gradient
            P.firstDerivative (gradient_, xtd_);
            // and it squared norm
            qpt_ = DotProduct (gradient_, gradient_);

            // Return new step value
            return t;
            }


    }

}
