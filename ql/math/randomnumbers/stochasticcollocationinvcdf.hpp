/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2016 Klaus Spanderen

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

/*! \file stochasticcollocationinvcdf.hpp
    Stochastic collocation inverse cumulative distribution function
*/

#ifndef quantlib_stochastic_collation_inv_cdf_hpp
#define quantlib_stochastic_collation_inv_cdf_hpp

#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/functional.hpp>

namespace QuantLib {
    //! Stochastic collocation inverse cumulative distribution function

    /*! References:
        L.A. Grzelak, J.A.S. Witteveen, M.Suárez-Taboada, C.W. Oosterlee,
        The Stochastic Collocation Monte Carlo Sampler: Highly efficient
        sampling from “expensive” distributions
        http://papers.ssrn.com/sol3/papers.cfm?abstract_id=2529691
     */

    class StochasticCollocationInvCDF {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        StochasticCollocationInvCDF(
            const ext::function<Real(Real)>& invCDF,
            Size lagrangeOrder,
            Real pMax = Null<Real>(),
            Real pMin = Null<Real>());

        Real value(Real x) const;
        Real operator()(Real u) const;

      private:
        const Array x_;
        const Volatility sigma_;
        const Array y_;
        const LagrangeInterpolation interpl_;
    };
}

#endif
