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

#ifndef quantlib_exponentail_integral_hpp
#define quantlib_exponentail_integral_hpp

#include <ql/types.hpp>
#include <complex>

#ifndef M_EULER_MASCHERONI
#define M_EULER_MASCHERONI 0.5772156649015328606065121
#endif

/*! References:
    B. Rowe et al: GALSIM: The modular galaxy image simulation toolkit
    https://arxiv.org/abs/1407.7676
    V. Pegoraro, P. Slusallek:
    On the Evaluation of the Complex-Valued Exponential Integral
    https://www.sci.utah.edu/~vpegorar/research/2011_JGT.pdf
*/
namespace QuantLib::ExponentialIntegral {
    
    Real Si(Real x);
    Real Ci(Real x);

    std::complex<Real> Ci(const std::complex<Real>& z);
    std::complex<Real> Si(const std::complex<Real>& z);
    std::complex<Real> E1(const std::complex<Real>& z);
    std::complex<Real> Ei(const std::complex<Real>& z);

}

#endif
