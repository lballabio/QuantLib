/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Allen Kuo

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

#include <ql/math/bspline.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    BSpline::BSpline(Natural p,
                     Natural n,
                     const std::vector<Real>& knots)
    : p_(p), n_(n), knots_(knots) {

        QL_REQUIRE(p >= 1, "lowest degree B-spline has p = 1");
        QL_REQUIRE(n >= 1, "number of control points n+1 >= 2");
        QL_REQUIRE(p <= n, "must have p <= n");

        QL_REQUIRE(knots.size() == p+n+2,"number of knots must equal p+n+2");

        for (Size i=0; i<knots.size()-1; ++i) {
            QL_REQUIRE(knots[i] <= knots[i+1],
                       "knots points must be nondecreasing");
        }
    }


    Real BSpline::operator()(Natural i, Real x) const {
        QL_REQUIRE(i <= n_, "i must not be greater than n");
        return N(i,p_,x);
    }


    Real BSpline::N(Natural i, Natural p, Real x) const {

        if (p==0) {
            return (knots_[i] <= x && x < knots_[i+1]) ? 1.0 : 0.0;
        } else {
            return ((x - knots_[i])/(knots_[i+p] - knots_[i]))*N(i,p-1,x) +
                ((knots_[i+p+1]-x)/(knots_[i+p+1]-knots_[i+1]))* N(i+1,p-1,x);
        }
    }

}

