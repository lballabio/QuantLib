/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Marek Glowacki

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

/*! \file gaussiancopula.hpp
    \brief gaussian copula
*/

#ifndef quantlib_math_gaussian_copula_h
#define quantlib_math_gaussian_copula_h

#include <ql/math/distributions/bivariatenormaldistribution.hpp>
#include <functional>

namespace QuantLib {

    //! Gaussian copula
    class GaussianCopula {
      public:
        typedef Real first_argument_type;
        typedef Real second_argument_type;
        typedef Real result_type;

        GaussianCopula(Real rho);
        Real operator()(Real x, Real y) const;
      private:
        Real rho_;
        BivariateCumulativeNormalDistributionWe04DP bivariate_normal_cdf_;     
        InverseCumulativeNormal invCumNormal_;
    };
    
}

#endif
