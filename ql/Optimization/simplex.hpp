/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file simplex.hpp
    \brief Simplex optimization method
*/

/* The implementation of the algorithm was inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, Flannery
 * Chapter 10
 */

#ifndef quantlib_optimization_simplex_hpp
#define quantlib_optimization_simplex_hpp

#include <ql/Optimization/problem.hpp>
#include <vector>

namespace QuantLib {

    //! Multi-dimensional simplex class
    class Simplex : public OptimizationMethod {
      public:
        /*! Constructor taking as input the characteristic length and
            tolerance
        */
        Simplex(Real lambda, Real tol)
        : lambda_(lambda), tol_(tol) {}
        virtual ~Simplex() {}

        virtual void minimize(const Problem& P) const;
      private:
        Real extrapolate(const Problem& P, Size iHighest,
                         Real& factor) const;
        Real lambda_;
        Real tol_;
        mutable std::vector<Array> vertices_;
        mutable Array values_, sum_;
    };

}


#endif
