/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2010 Kakhkhor Abdijalilov

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

/*! \file lsmbasissystem.hpp
    \brief utility classes for Longstaff-Schwartz early-exercise Monte Carlo
*/

// lsmbasissystem.hpp

#ifndef quantlib_lsm_basis_system_hpp
#define quantlib_lsm_basis_system_hpp

#include <ql/qldefines.hpp>
#include <ql/math/array.hpp>
#include <ql/functional.hpp>
#include <vector>

namespace QuantLib {

    class LsmBasisSystem {
      public:
        enum PolynomialType {
            Monomial, Laguerre, Hermite, Hyperbolic,
            Legendre, Chebyshev, Chebyshev2nd
        };

        static std::vector<std::function<Real(Real)> >
        pathBasisSystem(Size order, PolynomialType type);

        static std::vector<std::function<Real(Array)> >
        multiPathBasisSystem(Size dim, Size order, PolynomialType type);
    };


}

#endif
