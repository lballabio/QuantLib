/*
 * Copyright (C) 2001
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
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
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file avgstrikeasianpathpricer.cpp
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/02/13 15:05:24  lballabio
    Trimmed a couple of long file names for Mac compatibility

    Revision 1.1  2001/02/05 16:52:06  marmar
    AverageAsianPathPricer substituted by AveragePriceAsianPathPricer
    and AverageStrikeAsianPathPricer

*/

#include "avgstrikeasianpathpricer.h"

namespace QuantLib {

    namespace MonteCarlo {

        AverageStrikeAsianPathPricer::AverageStrikeAsianPathPricer(
          Option::Type type, double underlying, double strike, double discount)
        : EuropeanPathPricer(type, underlying, strike, discount) {
            isInitialized_=true;
        }

        double AverageStrikeAsianPathPricer::value(const Path & path) const {

            int n = path.size();
            QL_REQUIRE(n>0,"the path cannot be empty");
            QL_REQUIRE(isInitialized_,
                "AverageStrikeAsianPathPricer: pricer not initialized");

            double price = underlying_;
            double averageStrike = 0.0;

            for (int i=0; i<n; i++) {
                price *= QL_EXP(path[i]);
                averageStrike += price;
            }
            averageStrike = averageStrike/n;

            return computePlainVanilla(type_, price, averageStrike, discount_);
        }
        
    }

}
