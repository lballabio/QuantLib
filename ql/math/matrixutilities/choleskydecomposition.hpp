/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file choleskydecomposition.hpp
    \brief Cholesky decomposition
*/

#ifndef quantlib_cholesky_decomposition_hpp
#define quantlib_cholesky_decomposition_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    /*! \relates Matrix */
    Matrix CholeskyDecomposition(const Matrix& m, bool flexible = false);
    Array CholeskySolveFor(const Matrix& L, const Array& b);
}


#endif
