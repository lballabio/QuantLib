
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file lecuyeruniformrng.cpp
    \brief L'Ecuyer uniform random number generator

    \fullpath
    ql/RandomNumbers/%lecuyeruniformrng.cpp
*/

// $Id$

#include <ql/RandomNumbers/lecuyeruniformrng.hpp>

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
