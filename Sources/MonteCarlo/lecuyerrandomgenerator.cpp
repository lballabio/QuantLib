
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file lecuyerrandomgenerator.cpp
    \brief L'Ecuyer uniform random number generator

    $Source$
    $Name$
    $Log$
    Revision 1.6  2001/04/09 14:13:34  nando
    all the *.hpp moved below the Include/ql level

    Revision 1.5  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/04 12:13:24  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.3  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.1  2000/12/27 15:23:39  marmar
    Random number generators has been updated and documented.
    Now the Sample Generator idea is fully implemented

*/


#include "ql/MonteCarlo/lecuyerrandomgenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        const long LecuyerRandomGenerator::m1 = 2147483563L;
        const long LecuyerRandomGenerator::a1 = 40014L;
        const long LecuyerRandomGenerator::q1 = 53668L;
        const long LecuyerRandomGenerator::r1 = 12211L;

        const long LecuyerRandomGenerator::m2 = 2147483399L;
        const long LecuyerRandomGenerator::a2 = 40692L;
        const long LecuyerRandomGenerator::q2 = 52774L;
        const long LecuyerRandomGenerator::r2 = 3791L;

        const int LecuyerRandomGenerator::bufferSize = 32;
        const long LecuyerRandomGenerator::bufferNormalizer = 67108862L;             // 1+(m1-1)/bufferSize

        const long double LecuyerRandomGenerator::maxRandom = 1.0-QL_EPSILON;

    }
}
