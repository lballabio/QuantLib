/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Cheng Li
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

/*! \file goldstein.hpp
    \brief Goldstein and Price line-search class
*/

#ifndef quantlib_optimization_goldstein_hpp
#define quantlib_optimization_goldstein_hpp

#include <ql/math/optimization/linesearch.hpp>

namespace QuantLib {

    class GoldsteinLineSearch : public LineSearch {
      public:
        //! Default constructor
        GoldsteinLineSearch(Real eps = 1e-8,
                            Real alpha = 0.05,
                            Real beta = 0.65,
                            Real extrapolation = 1.5)
        : LineSearch(eps), alpha_(alpha), beta_(beta), extrapolation_(extrapolation) {}

        //! Perform line search
        Real operator()(Problem& P, // Optimization problem
                        EndCriteria::Type& ecType,
                        const EndCriteria&,
                        Real t_ini) override; // initial value of line-search step

      private:
        Real alpha_, beta_;
        Real extrapolation_;
    };

}

#endif


#ifndef id_de8b995afb9d7f50869b85fafe525567
#define id_de8b995afb9d7f50869b85fafe525567
inline bool test_de8b995afb9d7f50869b85fafe525567(int* i) { return i != 0; }
#endif
