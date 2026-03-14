/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2016 Klaus Spanderen

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

/*! \file stochasticcollationcdf.cpp
*/

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/math/randomnumbers/stochasticcollocationinvcdf.hpp>
#include <ql/mathconstants.hpp>

namespace QuantLib {

    namespace {
        Array g(Real sigma, const Array& x,
                const std::function<Real(Real)>& invCDF) {

            Array y(x.size());
            const CumulativeNormalDistribution normalCDF;

            for (Size i=0, n=x.size(); i < n; ++i) {
                y[i] = invCDF(normalCDF(x[i]/sigma));
            }

            return y;
        }
    }

    StochasticCollocationInvCDF::StochasticCollocationInvCDF(
        const std::function<Real(Real)>& invCDF,
        Size lagrangeOrder, Real pMax, Real pMin)
    : x_(M_SQRT2*GaussHermiteIntegration(lagrangeOrder).x()),
      sigma_( (pMax != Null<Real>())
              ? x_.back() / InverseCumulativeNormal()(pMax)
              : (pMin != Null<Real>())
                  ? Real(x_.front() / InverseCumulativeNormal()(pMin))
                  : 1.0),
      y_(g(sigma_, x_, invCDF)),
      interpl_(x_.begin(), x_.end(), y_.begin()) {
    }

    Real StochasticCollocationInvCDF::value(Real x) const {
        return interpl_(x*sigma_, true);
    }
    Real StochasticCollocationInvCDF::operator()(Real u) const {
        return value(InverseCumulativeNormal()(u));
    }
}
