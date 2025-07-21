/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file richardsonextrapolation.hpp
*/

#ifndef quantlib_richardson_extrapolation_hpp
#define quantlib_richardson_extrapolation_hpp

#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    //! Richardson Extrapolation
    /*! Richardson Extrapolation is a sequence acceleration technique for
      \f[
          f(\Delta h) = f_0 + \alpha\cdot (\Delta h)^n + O((\Delta h)^{n+1})
      \f]
     */

    /*! References:
        http://en.wikipedia.org/wiki/Richardson_extrapolation
     */

    class RichardsonExtrapolation {
      public:
        /*! Richardon Extrapolation
           \param f function to be extrapolated to delta_h -> 0
           \param delta_h step size
           \param n if known, n is the order of convergence
         */
        RichardsonExtrapolation(const std::function<Real (Real)>& f,
                                Real delta_h, Real n = Null<Real>());


        /*! Extrapolation for known order of convergence
            \param t scaling factor for the step size
        */
        Real operator()(Real t=2.0) const;

        /*! Extrapolation for unknown order of convergence
            \param t first scaling factor for the step size
            \param s second scaling factor for the step size
        */
        Real operator()(Real t, Real s) const;

      private:
        const Real delta_h_;
        const Real fdelta_h_;
        const Real n_;
        const std::function<Real (Real)> f_;
    };
}

#endif
