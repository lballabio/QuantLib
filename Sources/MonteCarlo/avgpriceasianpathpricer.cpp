
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

/*! \file avgpriceasianpathpricer.cpp

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/04/06 18:46:21  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.4  2001/04/04 13:32:54  enri
    tons of typos fixed

    Revision 1.3  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.2  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.1  2001/02/13 15:05:24  lballabio
    Trimmed a couple of long file names for Mac compatibility

    Revision 1.1  2001/02/05 16:52:06  marmar
    AverageAsianPathPricer substituted by AveragePriceAsianPathPricer
    and AverageStrikeAsianPathPricer

    Revision 1.2  2001/01/10 16:34:32  nando
    unsigned int < int turned into int < int to avoid warning

    Revision 1.1  2001/01/05 11:52:12  lballabio
    Renamed SinglePathAveragePriceAsianPricer to AveragePriceAsianPathPricer

    Revision 1.2  2001/01/05 11:42:38  lballabio
    Renamed SinglePathEuropeanPricer to EuropeanPathPricer

    Revision 1.1  2001/01/04 17:31:23  marmar
    Alpha version of the Monte Carlo tools.

*/

#include "MonteCarlo/avgpriceasianpathpricer.hpp"

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
