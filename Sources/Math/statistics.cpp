
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

/*! \file statistics.cpp

    \fullpath
    Sources/Math/%statistics.cpp
    \brief statistic tools

*/

// $Id$
// $Log$
// Revision 1.19  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.18  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.17  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.16  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.15  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.14  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Math/statistics.hpp"

namespace QuantLib {

    namespace Math {

        Statistics::Statistics() {
            reset();
        }

        void Statistics::reset() {
            min_ = QL_MAX_DOUBLE;
            max_ = QL_MIN_DOUBLE;
            sampleNumber_ = 0;
            sampleWeight_ = 0.0;
            sum_ = 0.0;
            quadraticSum_ = 0.0;
            downsideQuadraticSum_ = 0.0;
            cubicSum_ = 0.0;
            fourthPowerSum_ = 0.0;
        }

    }

}
