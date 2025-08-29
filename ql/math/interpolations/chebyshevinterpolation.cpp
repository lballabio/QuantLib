/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

#include <ql/math/interpolations/lagrangeinterpolation.hpp>
#include <ql/math/interpolations/chebyshevinterpolation.hpp>

namespace QuantLib {
    namespace chebyshev_interpolation_detail {
        Array apply(const Array& x, const std::function<Real(Real)>& f) {
            Array t(x.size());
            std::transform(std::begin(x), std::end(x), std::begin(t), f);

            return t;
        }
    }

    ChebyshevInterpolation::ChebyshevInterpolation(
        const Array& y, PointsType pointsType)
    : x_(ChebyshevInterpolation::nodes(y.size(), pointsType)), y_(y) {

        impl_ = ext::make_shared<detail::LagrangeInterpolationImpl<
            Array::const_iterator, Array::const_iterator> >(
            std::begin(x_), std::end(x_), std::begin(y_)
        );

        impl_->update();
    }

    ChebyshevInterpolation::ChebyshevInterpolation(
        Size n, const std::function<Real(Real)>& f, PointsType pointsType)
    : ChebyshevInterpolation(
          chebyshev_interpolation_detail::apply(
              ChebyshevInterpolation::nodes(n, pointsType), f),
              pointsType) {
    }


    Array ChebyshevInterpolation::nodes() const {
        return x_;
    }

    Array ChebyshevInterpolation::nodes(Size n, PointsType pointsType) {
        Array t(n);

        switch(pointsType) {
          case FirstKind:
            for (Size i=0; i < n; ++i)
                t[i] = -std::cos((i+0.5)*M_PI/n);
            break;
          case SecondKind:
            for (Size i=0; i < n; ++i)
                t[i] = -std::cos(i*M_PI/(n-1));
            break;
          default:
              QL_FAIL("unknonw Chebyshev interpolation points type");
        }
        return t;
    }

    void ChebyshevInterpolation::updateY(const Array& y) {
        QL_REQUIRE(y.size() == y_.size(),
            "interpolation override has the wrong length");

        std::copy(y.begin(), y.end(), y_.begin());
    }
}

