/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Klaus Spanderen

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

#include <ql/math/expm1.hpp>
#include <ql/math/functional.hpp>

#include <cmath>

namespace QuantLib {
    std::complex<Real> expm1(const std::complex<Real>& z) {
        if (std::abs(z) < 1.0) {
            const Real a = z.real(), b = z.imag();
            const Real exp_1 = std::expm1(a);
            const Real cos_1 = -2*squared(std::sin(0.5*b));

            return std::complex<Real>(
                exp_1*cos_1 + exp_1 + cos_1,
                std::sin(b)*std::exp(a)
            );
        }
        else {
            return std::exp(z)-1.0;
        }
    }

    std::complex<Real> log1p(const std::complex<Real>& z) {
        const Real a = z.real(), b = z.imag();
        if (std::abs(a) < 0.5 && std::abs(b) < 0.5) {
            return std::complex<Real>(
                0.5*std::log1p(a*a + 2*a + b*b),
                std::arg(1.0 + z)
            );
        }
        else {
            return std::log(1.0+z);
        }
    }
}
