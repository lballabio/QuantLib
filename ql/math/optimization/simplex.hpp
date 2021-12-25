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
    /*! This method is rather raw and requires quite a lot of
        computing resources, but it has the advantage that it does not
        need any evaluation of the cost function's gradient, and that
        it is quite easily implemented. First, we choose N+1
        starting points, given here by a starting point \f$
        \mathbf{P}_{0} \f$ and N points such that
        \f[ 
            \mathbf{P}_{\mathbf{i}}=\mathbf{P}_{0}+\lambda \mathbf{e}_{\mathbf{i}},
        \f]
        where \f$ \lambda \f$ is the problem's characteristic length scale). These 
        points will form a geometrical form called simplex.
        The principle of the downhill simplex method is, at each
        iteration, to move the worst point (highest cost function value)
        through the opposite face to a better point. When the simplex
        seems to be constrained in a valley, it will be contracted
        downhill, keeping the best point unchanged.

        \ingroup optimizers
    */
    class Simplex : public OptimizationMethod {
      public:
        /*! Constructor taking as input the characteristic length */
        Simplex(Real lambda) : lambda_(lambda) {}
        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;
        Real lambda() const { return lambda_; }

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
