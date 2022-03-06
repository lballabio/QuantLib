/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Klaus Spanderen

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

/*! \file exponentialintegrals.hpp
*/

#include <ql/types.hpp>

#include <complex>

namespace QuantLib {
    /*! References:

        B. Rowe et al: GALSIM: The modular galaxy image simulation toolkit
        https://arxiv.org/abs/1407.7676
    */
    namespace ExponentialIntegral {
        Real Si(Real x);
        Real Ci(Real x);

        std::complex<Real> Ci(std::complex<Real> z);
        std::complex<Real> Si(std::complex<Real> z);
        std::complex<Real> E1(std::complex<Real> z);
        std::complex<Real> Ei(std::complex<Real> z);
    }
}


#ifndef id_dccfb4c0ce852012c650cbbbba56e0d3
#define id_dccfb4c0ce852012c650cbbbba56e0d3
inline bool test_dccfb4c0ce852012c650cbbbba56e0d3(int* i) { return i != 0; }
#endif
