/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file simplex.hpp
    \brief Simplex optimization method

    \fullpath
    ql/Optimization/%simplex.hpp
*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 10
 */

#ifndef quantlib_optimization_simplex_h
#define quantlib_optimization_simplex_h

#include <ql/Optimization/optimizer.hpp>

#include <vector>

namespace QuantLib {

    namespace Optimization {

        //! Multi-dimensionnal Simplex class
        class Simplex : public OptimizationMethod {
          public:
            Simplex(double lambda, double tol) 
            : OptimizationMethod(), lambda_(lambda), tol_(tol) {}
            virtual ~Simplex() {}

            double extrapolate(OptimizationProblem& P, Size iHighest, 
                               double factor);

            //! minimize the optimization problem P
            virtual void minimize(OptimizationProblem& P);
          private:
            double lambda_;
            double tol_;
            std::vector<Array> vertices_;
            Array values_;
            Array sum_;
        };

    }

}


#endif
