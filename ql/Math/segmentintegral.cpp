
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

/*! \file segmentintegral.cpp
    \brief Integral of a function over a segment

    \fullpath
    ql/Math/%segmentintegral.cpp
*/

// $Id$

#include "ql/Math/segmentintegral.hpp"

namespace QuantLib {

    namespace Math {

        SegmentIntegral::SegmentIntegral(unsigned int intervals)
            :intervals_(intervals){
            QL_REQUIRE(intervals > 3,
               "at least 4 intervals needed, given only "+
               IntegerFormatter::toString(intervals));
            }

        double SegmentIntegral::operator()(
            const ObjectiveFunction& f, double a, double b) const {
                QL_REQUIRE(a < b,
                           "to compute an integral on [a,b] it must be a<b; a="+
                           DoubleFormatter::toString(a)+" b="+
                           DoubleFormatter::toString(b));

                double dx = (b-a)/intervals_;
                double sum = 0.5 * f(a + 0.5 * dx) * dx;
                sum += 0.5 * f(b - 0.5 * dx) * dx;

                for(double x = a + 1.5 * dx; x < b - 0.5 * dx; x += dx)
                    sum += f(x) * dx;

                return sum;
        }

    }

}
