
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

/*! \file geometricasopathpricer.cpp
    \brief path pricer for geometric average strike option

    \fullpath
    ql/MonteCarlo/%geometricasopathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/geometricasopathpricer.hpp"
#include "ql/Pricers/singleassetoption.hpp"



using QuantLib::Pricers::ExercisePayoff;

namespace QuantLib {

    namespace MonteCarlo {

        GeometricASOPathPricer::GeometricASOPathPricer(Option::Type type,
            double underlying, double discount, bool antitheticVariance)
        : SingleAssetPathPricer(type, underlying, underlying, discount,
          antitheticVariance) {}

        double GeometricASOPathPricer::operator()(const Path& path) const {

            int n = path.size();
            QL_REQUIRE(n>0,"GeometricASOPathPricer: the path cannot be empty");

            double price1 = underlying_;
            double averageStrike1 = 1.0;
            int i;
            for (i=0; i<n; i++) {
                price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                averageStrike1 *= price1;
            }
            averageStrike1 = QL_POW(averageStrike1, 1.0/n);

            if (antitheticVariance_) {
                double price2 = underlying_;
                double averageStrike2 = 1.0;

                for (i=0; i<n; i++) {
                    price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                    averageStrike2 *= price2;
                }
                averageStrike2 = QL_POW(averageStrike2, 1.0/n);
                return discount_/2.0*(ExercisePayoff(type_, price1, averageStrike1)
                    +ExercisePayoff(type_, price2, averageStrike2));
            } else
                return discount_*ExercisePayoff(type_, price1, averageStrike1);
        }

    }

}
