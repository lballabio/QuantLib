
/*
 * Copyright (C) 2000
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
          if(!hasBeenCalculated) {
            growthDiscount = (QL_EXP(-theUnderlyingGrowthRate*theResidualTime));
            riskFreeDiscount = (QL_EXP(-theRiskFreeRate*theResidualTime));
              standardDeviation = theVolatility*QL_SQRT(theResidualTime);
              Math::CumulativeNormalDistribution f;
              double D1 = QL_LOG(theUnderlying/theStrike)/standardDeviation + standardDeviation/2.0
                  + (theRiskFreeRate-theUnderlyingGrowthRate)*theResidualTime/standardDeviation;
              double D2 = D1 - standardDeviation;
              double fD1 = f(D1), fD2 = f(D2);
              switch (theType) {
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
                  throw IllegalArgumentError("BSMEuropeanOption: invalid option type");
            }
            hasBeenCalculated = true;
            theValue = theUnderlying*growthDiscount*alpha - theStrike*riskFreeDiscount*beta;
          }
            return theValue;
        }

        double BSMEuropeanOption::delta() const {
          if(!hasBeenCalculated)
            value();

            return growthDiscount*alpha;
        }

        double BSMEuropeanOption::gamma() const {
          if(!hasBeenCalculated)
            value();

            return NID1*growthDiscount/(theUnderlying*standardDeviation);
        }

        double BSMEuropeanOption::theta() const {
          if(!hasBeenCalculated)
            value();

            return -theUnderlying*NID1*theVolatility*growthDiscount/(2.0*QL_SQRT(theResidualTime)) +
                  theUnderlyingGrowthRate*theUnderlying*alpha*growthDiscount - theRiskFreeRate*theStrike*riskFreeDiscount*beta;
        }

        double BSMEuropeanOption::rho() const {
          if(!hasBeenCalculated)
            value();

            return theResidualTime*riskFreeDiscount*theStrike*beta;
        }

        double BSMEuropeanOption::vega() const {
          if(!hasBeenCalculated)
            value();

            return theUnderlying*NID1*growthDiscount*QL_SQRT(theResidualTime);
        }

    }

}
