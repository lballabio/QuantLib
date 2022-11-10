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

/*! \file tanhsinhintegral.hpp
*/

#ifndef quantlib_tanh_sinh_integral_hpp
#define quantlib_tanh_sinh_integral_hpp

#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/integrals/integral.hpp>

#if BOOST_VERSION >= 106900
#define QL_BOOST_HAS_TANH_SINH

#include <boost/math/quadrature/tanh_sinh.hpp>
#include <limits>

namespace QuantLib {
    using tanh_sinh = boost::math::quadrature::tanh_sinh<Real>;

    /*! The tanh-sinh quadrature routine provided by boost is a rapidly convergent
        numerical integration scheme for holomorphic integrands. The tolerance
        is used against the error estimate for the L1 norm of the integral.
    */
    class TanhSinhIntegral : public Integrator {
      public:
        TanhSinhIntegral(
            Real relTolerance = std::sqrt(std::numeric_limits<Real>::epsilon()),
            Size maxRefinements = 15,
            Real minComplement = std::numeric_limits<Real>::min() * 4
            )
      : Integrator(QL_MAX_REAL, Null<Size>()),
        relTolerance_(relTolerance),
        tanh_sinh_(maxRefinements, minComplement) {}

      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b)
        const override {
            Real error;
            Real value = tanh_sinh_.integrate(f, a, b, relTolerance_, &error);
            setAbsoluteError(error);

            return value;
        }

      private:
        const Real relTolerance_;
        mutable tanh_sinh tanh_sinh_;
    };
}

#else

namespace QuantLib {
    class TanhSinhIntegral : public Integrator {
      public:
        TanhSinhIntegral(Size m = 0, Real a = 0, Real b = 0)
        : Integrator(Null<Real>(), Null<Size>()) {
            QL_FAIL("boost version 1.66 or higher is required to "
                    "use TanhSinhIntegraion");
        }

      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b)
        const override { return 0.0; }
    };
}

#endif

#endif
