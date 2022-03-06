/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Klaus Spanderen

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

/*! \file expm.hpp
    \brief matrix exponential
*/

#ifndef quantlib_expm_hpp
#define quantlib_expm_hpp

#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! matrix exponential based on the ordinary differential equations method

    /*! References:

        C. Moler; C. Van Loan, 1978,
        Nineteen Dubious Ways to Compute the Exponential of a Matrix
        http://xa.yimg.com/kq/groups/22199541/1399635765/name/moler-nineteen.pdf
    */

    //! returns the matrix exponential exp(t*M)
    Disposable<Matrix> Expm(const Matrix& M, Real t=1.0, Real tol=QL_EPSILON);
}

#endif


#ifndef id_929bb9f91af8a1c73ff3f77434b67810
#define id_929bb9f91af8a1c73ff3f77434b67810
inline bool test_929bb9f91af8a1c73ff3f77434b67810(int* i) { return i != 0; }
#endif
