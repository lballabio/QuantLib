/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

#include <ql/math/matrixutilities/householder.hpp>

namespace QuantLib {

    HouseholderTransformation::HouseholderTransformation(Array v)
    : v_(std::move(v)) {}


    Array HouseholderTransformation::operator()(const Array& x) const {
        return x - (2.0*DotProduct(v_, x))*v_;
    }

    Matrix HouseholderTransformation::getMatrix() const {
        const Array y = v_ / Norm2(v_);
        const Size n = y.size();

        Matrix m(n, n);
        for (Size i=0; i < n; ++i)
            for (Size j=0; j < n; ++j)
                m[i][j] = ((i == j)? 1.0: 0.0) - 2*y[i]*y[j];

        return m;
    }

    HouseholderReflection::HouseholderReflection(Array e)
    : e_(std::move(e)) {}

    Array HouseholderReflection::reflectionVector(const Array& a) const {
        const Real na = Norm2(a);
        QL_REQUIRE(na > 0, "vector of length zero given");

        const Real aDotE = DotProduct(a, e_);
        const Array a1 = aDotE*e_;
        const Array a2 = a - a1;

        const Real eps = DotProduct(a2, a2) / (aDotE*aDotE);
        if (eps < QL_EPSILON*QL_EPSILON) {
            return Array(a.size(), 0.0);
        }
        else if (eps < 1e-4) {
            const Real eps2 = eps*eps;
            const Real eps3 = eps*eps2;
            const Real eps4 = eps2*eps2;
            const Array v =
                (a2 - a1*(eps/2.0 - eps2/8.0 + eps3/16.0 - 5/128.0*eps4))
                / (aDotE*std::sqrt(eps + eps2/4.0 - eps3/8.0 + 5/64.0*eps4));
            return v;
        }
        else {
            const Array c = a - na*e_;
            return c / Norm2(c);
        }
    }

    Array HouseholderReflection::operator()(const Array& a) const {
        const Array v = reflectionVector(a);
        return HouseholderTransformation(v)(a);
    }
}
