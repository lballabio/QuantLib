
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

/*! \file avgpriceasianpathpricer.cpp
    \brief path pricer for average price Asian option

    \fullpath
    ql/MonteCarlo/%avgpriceasianpathpricer.cpp

*/

// $Id$

#include "ql/MonteCarlo/avgpriceasianpathpricer.hpp"
#include "ql/Pricers/singleassetoption.hpp"



using QuantLib::Pricers::ExercisePayoff;

namespace QuantLib {

    namespace MonteCarlo {

        AveragePriceAsianPathPricer::AveragePriceAsianPathPricer(
          Option::Type type, double underlying, double strike, double discount,
          bool antitheticVariance)
        : SingleAssetPathPricer(type, underlying, strike, discount,
          antitheticVariance) {}

        double AveragePriceAsianPathPricer::operator()(const Path& path) const {

            int n = path.size();
            QL_REQUIRE(n>0,
                "AveragePriceAsianPathPricer: the path cannot be empty");

            double price1 = underlying_;
            double averagePrice1 = 0.0;
            int i;
            for (i=0; i<n; i++) {
                price1 *= QL_EXP(path.drift()[i]+path.diffusion()[i]);
                averagePrice1 += price1;
            }
            averagePrice1 = averagePrice1/n;

            if (antitheticVariance_) {
                double price2 = underlying_;
                double averagePrice2 = 0.0;

                for (i=0; i<n; i++) {
                    price2 *= QL_EXP(path.drift()[i]-path.diffusion()[i]);
                    averagePrice2 += price2;
                }
                averagePrice2 = averagePrice2/n;
                return discount_/2.0*(ExercisePayoff(type_, averagePrice1, strike_)
                    +ExercisePayoff(type_, averagePrice2, strike_));
            } else
                return discount_*ExercisePayoff(type_, averagePrice1, strike_);
        }

    }

}
