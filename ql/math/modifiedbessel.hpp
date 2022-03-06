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

/*! \file modifiedbessel.hpp
    \brief modified Bessel functions of first and second kind
*/

#ifndef quantlib_modified_bessel_hpp
#define quantlib_modified_bessel_hpp

#include <ql/types.hpp>
#include <complex>

namespace QuantLib {

    /*  Compute modified Bessel functions I_nv(x) and K_nv(x)

        Based on series expansion outlined in e.g.
        http://www.mhtlab.uwaterloo.ca/courses/me755/web_chap4.pdf

        The exponentially weighted versions return the function value
        times exp(-x) resp exp(-z)
     */
    Real modifiedBesselFunction_i(Real nu, Real x);
    Real modifiedBesselFunction_k(Real nu, Real x);
    Real modifiedBesselFunction_i_exponentiallyWeighted(Real nu, Real x);
    Real modifiedBesselFunction_k_exponentiallyWeighted(Real nu, Real x);

    std::complex<Real> modifiedBesselFunction_i(Real nu,
                                                const std::complex<Real>& z);
    std::complex<Real> modifiedBesselFunction_k(Real nu,
                                                const std::complex<Real>& z);
    std::complex<Real>
    modifiedBesselFunction_i_exponentiallyWeighted(Real nu,
                                                   const std::complex<Real> &z);
    std::complex<Real>
    modifiedBesselFunction_k_exponentiallyWeighted(Real nu,
                                                  const std::complex<Real> &z);
}
#endif


#ifndef id_8a1da29977b9f6c76e98e64ab64ae8a3
#define id_8a1da29977b9f6c76e98e64ab64ae8a3
inline bool test_8a1da29977b9f6c76e98e64ab64ae8a3(int* i) { return i != 0; }
#endif
