
/*
 * Copyright (C) 2000, 2001
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file bsmeuropeanoption.cpp
    \brief european option

    $Source$
    $Name$
    $Log$
    Revision 1.21  2001/02/19 12:19:29  marmar
    Added trailing _ to protected and private members

    Revision 1.20  2001/02/13 10:02:57  marmar
    Ambiguous variable name underlyingGrowthRate changed in
    unambiguos dividendYield

    Revision 1.19  2001/01/17 14:37:57  nando
    tabs removed

    Revision 1.18  2000/12/14 12:40:14  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#include "bsmeuropeanoption.h"
#include "normaldistribution.h"

namespace QuantLib {

    namespace Pricers {

        double BSMEuropeanOption::value() const {
          if(!hasBeenCalculated_) {
            growthDiscount = (QL_EXP(-dividendYield_*residualTime_));
            riskFreeDiscount = (QL_EXP(-riskFreeRate_*residualTime_));
              standardDeviation = volatility_*QL_SQRT(residualTime_);
              Math::CumulativeNormalDistribution f;
              double D1 = QL_LOG(underlying_/strike_)/standardDeviation + 
                    standardDeviation/2.0 + (riskFreeRate_ - dividendYield_)* 
                                            residualTime_/standardDeviation;
              double D2 = D1 - standardDeviation;
              double fD1 = f(D1), fD2 = f(D2);
              switch (type_) {
                  case Call:
                  alpha = fD1;
                  beta = fD2;
                  NID1 = f.derivative(D1);
                  break;
                  case Put:
                  alpha = fD1-1.0;
                  beta = fD2-1.0;
                  NID1 = f.derivative(D1);
                  break;
                  case Straddle:
                  alpha = 2.0*fD1-1.0;
                  beta = 2.0*fD2-1.0;
                  NID1 = 2.0*f.derivative(D1);
                  break;
                  default:
                  throw IllegalArgumentError(
                    "BSMEuropeanOption: invalid option type");
            }
            hasBeenCalculated_ = true;
            value_ = underlying_ * growthDiscount * alpha - 
                                    strike_ * riskFreeDiscount * beta;
          }
            return value_;
        }

        double BSMEuropeanOption::delta() const {
          if(!hasBeenCalculated_)
            value();

            return growthDiscount*alpha;
        }

        double BSMEuropeanOption::gamma() const {
          if(!hasBeenCalculated_)
            value();

            return NID1*growthDiscount/(underlying_*standardDeviation);
        }

        double BSMEuropeanOption::theta() const {
          if(!hasBeenCalculated_)
            value();

            return -underlying_ * NID1 * volatility_ * 
                growthDiscount/(2.0*QL_SQRT(residualTime_)) +
                  dividendYield_*underlying_*alpha*growthDiscount - 
                        riskFreeRate_*strike_*riskFreeDiscount*beta;
        }

        double BSMEuropeanOption::rho() const {
          if(!hasBeenCalculated_)
            value();

            return residualTime_*riskFreeDiscount*strike_*beta;
        }

        double BSMEuropeanOption::vega() const {
          if(!hasBeenCalculated_)
            value();

            return underlying_*NID1*growthDiscount*QL_SQRT(residualTime_);
        }

    }

}
