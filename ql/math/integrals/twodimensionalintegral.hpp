/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file twodimensionalintegral.hpp
    \brief two dimensional integration
*/

#ifndef quantlib_two_dimensional_integral_2d_hpp
#define quantlib_two_dimensional_integral_2d_hpp

#include <ql/math/integrals/integral.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/functional.hpp>
#include <utility>

namespace QuantLib {

    //! Integral of a two-dimensional function
    /*! The integral of a two dimensional function\f$ f(x,y) \f$
     *  between \f$ (a_x, a_y) \f$ and \f$ (b_x, b_y) \f$
        is calculated by means of two nested integrations
    */

    class TwoDimensionalIntegral {
      public:
        TwoDimensionalIntegral(ext::shared_ptr<Integrator> integratorX,
                               ext::shared_ptr<Integrator> integratorY)
        : integratorX_(std::move(integratorX)), integratorY_(std::move(integratorY)) {}

        Real operator()(const std::function<Real (Real, Real)>& f,
                        const std::pair<Real, Real>& a,
                        const std::pair<Real, Real>& b) const {
            return (*integratorX_)([&](Real x) { return g(f, x, a.second, b.second); },
                                   a.first, b.first);
        }

      private:
        Real g(const std::function<Real (Real, Real)>& f,
               Real x, Real a, Real b) const {
            return (*integratorY_)([&](Real y) { return f(x, y); }, a, b);
        }

        const ext::shared_ptr<Integrator> integratorX_, integratorY_;
    };
}
#endif
