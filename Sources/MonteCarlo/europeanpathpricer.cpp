
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

/*! \file europeanpathpricer.cpp

    $Id$
*/

// $Source$
// $Log$
// Revision 1.9  2001/07/02 12:36:18  sigmud
// pruned redundant header inclusions
//
// Revision 1.8  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/MonteCarlo/europeanpathpricer.hpp"


namespace QuantLib {

    namespace MonteCarlo {

        EuropeanPathPricer::EuropeanPathPricer(Option::Type type,
          double underlying, double strike, double discount)
        : type_(type),underlying_(underlying), strike_(strike),
          discount_(discount) {
            QL_REQUIRE(strike_ > 0.0,
                "SinglePathEuropeanPricer: strike must be positive");
            QL_REQUIRE(underlying_ > 0.0,
                "SinglePathEuropeanPricer: underlying must be positive");
            QL_REQUIRE(discount_ > 0.0,
                "SinglePathEuropeanPricer: discount must be positive");
            isInitialized_ = true;
        }

        double EuropeanPathPricer::value(const Path & path) const {
            int n = path.size();
            QL_REQUIRE(isInitialized_,
                "SinglePathEuropeanPricer: pricer not initialized");
            QL_REQUIRE(n>0,
                "SinglePathEuropeanPricer: the path cannot be empty");

            double price = underlying_;
            for(int i = 0; i < n; i++)
                price *= QL_EXP(path[i]);

            return computePlainVanilla(type_, price, strike_, discount_);
        }

        double EuropeanPathPricer::computePlainVanilla(
          Option::Type type, double price, double strike,
          double discount) const {
            double optionPrice;
            switch (type) {
              case Option::Call:
                    optionPrice = QL_MAX(price-strike,0.0);
                break;
              case Option::Put:
                    optionPrice = QL_MAX(strike-price,0.0);
                break;
              case Option::Straddle:
                    optionPrice = QL_FABS(strike-price);
            }
            return discount*optionPrice;
        }

    }

}

