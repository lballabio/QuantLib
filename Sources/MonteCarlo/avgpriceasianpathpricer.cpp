
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file avgpriceasianpathpricer.cpp
    \brief path pricer for average price Asian option

    $Id$
*/

// $Source$
// $Log$
// Revision 1.10  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.9  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.8  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/avgpriceasianpathpricer.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        AveragePriceAsianPathPricer::AveragePriceAsianPathPricer(
          Option::Type type, double underlying, double strike, double discount)
        : EuropeanPathPricer(type, underlying, strike, discount) {
            isInitialized_=true;
        }

        double AveragePriceAsianPathPricer::value(const Path & path) const {

            int n = path.size();
            QL_REQUIRE(n>0,"the path cannot be empty");
            QL_REQUIRE(isInitialized_,
                "AveragePriceAsianPathPricer: pricer not initialized");

            double price = underlying_;
            double averagePrice = 0.0;

            for (int i=0; i<n; i++) {
                price *= QL_EXP(path[i]);
                averagePrice += price;
            }
            averagePrice = averagePrice/n;

            return computePlainVanilla(type_, averagePrice, strike_, discount_);
        }

    }

}
