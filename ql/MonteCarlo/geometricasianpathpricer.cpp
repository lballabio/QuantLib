
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

/*! \file geometricasianpathpricer.cpp
    \brief path pricer for geometric Asian options

    \fullpath
    ql/MonteCarlo/%geometricasianpathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/geometricasianpathpricer.hpp"
#include "ql/Pricers/singleassetoption.hpp"



using QuantLib::Pricers::ExercisePayoff;

namespace QuantLib {

    namespace MonteCarlo {

        GeometricAsianPathPricer::GeometricAsianPathPricer(Option::Type type,
            double underlying, double strike, double discount,
            bool antitheticVariance)
        : SingleAssetPathPricer(type, underlying, strike, discount,
          antitheticVariance) {}

        double GeometricAsianPathPricer::operator()(const Path& path) const {

            int n = path.size();
            QL_REQUIRE(n>0,"the path cannot be empty");

            int i;
            double growth1 = 0.0;
            for (i=0; i<n; i++)
                growth1 += (n-i)*(path.drift()[i]+path.diffusion()[i]);
            double average1 = underlying_*QL_EXP(growth1/n);

            if (antitheticVariance_) {
                double growth2 = 0.0;
                for (i=0; i<n; i++)
                    growth2 += (n-i)*(path.drift()[i]-path.diffusion()[i]);
                double average2 = underlying_*QL_EXP(growth2/n);
                return discount_/2.0*(ExercisePayoff(type_, average1, strike_)
                    +ExercisePayoff(type_, average2, strike_));
            } else
                return discount_*ExercisePayoff(type_, average1, strike_);
        }

    }

}
