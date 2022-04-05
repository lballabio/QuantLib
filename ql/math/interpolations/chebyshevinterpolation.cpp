/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

#include <ql/math/interpolations/chebyshevinterpolation.hpp>

namespace QuantLib {
    ChebyshevInterpolation::ChebyshevInterpolation(const Array& f)
    : n_(f.size()), nodes_(ChebyshevInterpolation::nodes(n_)), a_(n_) {

        for (Size k=0; k < n_; ++k) {
            a_[k] = 0.5*(f[0]*std::pow(-1, int(k)) + f[n_-1]);

            for (Size i=1; i < n_-1; ++i) {
                a_[k] += f[i]*std::cos(k*M_PI*(1-i/Real(n_-1)));
            }

            a_[k] *= 2.0/(n_-1);
        }
    }

    ChebyshevInterpolation::ChebyshevInterpolation(
        Size n, const ext::function<Real(Real)>& f)
    : ChebyshevInterpolation(
          ChebyshevInterpolation::apply(
              ChebyshevInterpolation::nodes(n), f)) {}

    Real ChebyshevInterpolation::operator()(Real z) const {
        Real b_kp1 = a_[n_-1];
        Real b_kp2 = 0.0;

        for (Size k=n_-2; k != 0; --k) {
            const Real b = a_[k] + 2*z*b_kp1 - b_kp2;
            b_kp2 = b_kp1;
            b_kp1 = b;
        }

        return 0.5*a_[0] + z*b_kp1 - b_kp2;
    }

    Array ChebyshevInterpolation::nodes(Size n) {
        Array t(n);
        for (Size i=0; i < n; ++i) {
            t[i] = -std::cos(i*M_PI/(n-1));
        }

        return t;
    }

    Array ChebyshevInterpolation::apply(
        const Array& x, const ext::function<Real(Real)>& f) {
        Array t(x.size());
        std::transform(std::begin(x), std::end(x), std::begin(t), f);

        return t;
    }
}

