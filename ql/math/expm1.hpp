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

 /*! \file expm1.hpp
    \brief complex versions of expm1 and logp1
*/

#ifndef quantlib_expm1_hpp
#define quantlib_expm1_hpp

#include <ql/types.hpp>
#include <complex>

namespace QuantLib {
    std::complex<Real> expm1(const std::complex<Real>& z);
    std::complex<Real> log1p(const std::complex<Real>& z);
}
#endif

