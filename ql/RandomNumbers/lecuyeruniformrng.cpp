
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file lecuyeruniformrng.cpp
    \brief L'Ecuyer uniform random number generator

    \fullpath
    ql/RandomNumbers/%lecuyeruniformrng.cpp
*/

// $Id$

#include "ql/RandomNumbers/lecuyeruniformrng.hpp"

namespace QuantLib {

    namespace RandomNumbers {

        const long LecuyerUniformRng::m1 = 2147483563L;
        const long LecuyerUniformRng::a1 = 40014L;
        const long LecuyerUniformRng::q1 = 53668L;
        const long LecuyerUniformRng::r1 = 12211L;

        const long LecuyerUniformRng::m2 = 2147483399L;
        const long LecuyerUniformRng::a2 = 40692L;
        const long LecuyerUniformRng::q2 = 52774L;
        const long LecuyerUniformRng::r2 = 3791L;

        const int LecuyerUniformRng::bufferSize = 32;
        const long LecuyerUniformRng::bufferNormalizer = 67108862L;             // 1+(m1-1)/bufferSize

        const long double LecuyerUniformRng::maxRandom = 1.0-QL_EPSILON;

    }
}
