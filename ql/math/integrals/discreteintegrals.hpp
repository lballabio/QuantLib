/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file discreteintegrals.hpp
    \brief integrals on non uniform grids
*/

#ifndef quantlib_discrete_integrals_hpp
#define quantlib_discrete_integrals_hpp

#include <ql/math/array.hpp>
#include <ql/math/integrals/integral.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    /*! References:
        Levy, D. Numerical Integration
        http://www2.math.umd.edu/~dlevy/classes/amsc466/lecture-notes/integration-chap.pdf
    */
    class DiscreteTrapezoidIntegral {
      public:
        Real operator()(const Array& x, const Array& f) const;
    };

    class DiscreteSimpsonIntegral {
      public:
        Real operator()(const Array& x, const Array& f) const;
    };

    class DiscreteTrapezoidIntegrator: public Integrator {
      public:
        explicit DiscreteTrapezoidIntegrator(Size evaluations)
        : Integrator(Null<Real>(), evaluations) {}

      protected:
        Real integrate(const std::function<Real(Real)>& f, Real a, Real b) const override;
    };

    class DiscreteSimpsonIntegrator: public Integrator {
      public:
        explicit DiscreteSimpsonIntegrator(Size evaluations)
        : Integrator(Null<Real>(), evaluations) {}

      protected:
        Real integrate(const std::function<Real(Real)>& f, Real a, Real b) const override;
    };
}
#endif
