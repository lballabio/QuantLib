
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

/*! \file avgstrikeasianpathpricer.cpp

    \fullpath
    Sources/MonteCarlo/%avgstrikeasianpathpricer.cpp
    \brief path pricer for average strike Asian options

*/

// $Id$
// $Log$
// Revision 1.17  2001/08/31 15:23:47  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.16  2001/08/21 14:21:23  nando
// removed default constructors and useless isInitialized_ private member
//
// [also enabled MS Visual C++ profiling]
//
// Revision 1.15  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.14  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.13  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.12  2001/08/06 16:49:17  nando
// 1) BSMFunction now is VolatilityFunction
// 2) Introduced ExercisePayoff (to be reworked later)
//
// Revision 1.11  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.10  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.9  2001/07/19 16:40:11  lballabio
// Improved docs a bit
//
// Revision 1.8  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/avgstrikeasianpathpricer.hpp"
#include "ql/Pricers/singleassetoption.hpp"



using QuantLib::Pricers::ExercisePayoff;


namespace QuantLib {

    namespace MonteCarlo {

        AverageStrikeAsianPathPricer::AverageStrikeAsianPathPricer(
          Option::Type type, double underlying, double strike, double discount)
        : EuropeanPathPricer(type, underlying, strike, discount) {}

        double AverageStrikeAsianPathPricer::value(const Path & path) const {

            int n = path.size();
            QL_REQUIRE(n>0,"the path cannot be empty");

            double price = underlying_;
            double averageStrike = 0.0;

            for (int i=0; i<n; i++) {
                price *= QL_EXP(path[i]);
                averageStrike += price;
            }
            averageStrike = averageStrike/n;

            return discount_*ExercisePayoff(type_, price, averageStrike);
        }

    }

}
