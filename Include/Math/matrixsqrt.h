/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/
/*! \file matrixsqrt.h
    \brief Returns the square root of a real symmetric matrix

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/24 13:01:51  marmar
    Computes the square root of a square symmmetric real matrix with
    non-negative eigenvalues

*/


#ifndef quantlib_math_matrix_square_root_h
#define quantlib_math_matrix_square_root_h

#include "matrix.h"

namespace QuantLib {

    namespace Math {

        //!Returns the square root of a real symmetric matrix
        Matrix sqrt(const Matrix &realSymmMatrix);

    }

}

#endif
