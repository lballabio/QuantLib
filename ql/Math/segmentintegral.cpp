

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file segmentintegral.cpp
    \brief Integral of a function over a segment

    \fullpath
    ql/Math/%segmentintegral.cpp
*/

// $Id$

#include <ql/Math/segmentintegral.hpp>

namespace QuantLib {

    namespace Math {

        SegmentIntegral::SegmentIntegral(Size intervals)
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
