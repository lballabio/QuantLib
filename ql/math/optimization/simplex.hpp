/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

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

#include <ql/math/optimization/problem.hpp>
#include <vector>

namespace QuantLib {

    //! Multi-dimensional simplex class
    class Simplex : public OptimizationMethod {
      public:
        /*! Constructor taking as input the characteristic length */
        Simplex(Real lambda) : lambda_(lambda) {}
        virtual EndCriteria::Type minimize(Problem& P,
                                           const EndCriteria& endCriteria);
      private:
        Real extrapolate(Problem& P,
                         Size iHighest,
                         Real& factor) const;
        Real lambda_;
        mutable std::vector<Array> vertices_;
        mutable Array values_, sum_;
    };

}

#endif
