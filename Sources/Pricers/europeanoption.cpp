
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

/*! \file europeanoption.cpp
    \fullpath Sources/Pricers/%europeanoption.cpp
    \brief european option

    $Id$
*/

// $Log$
// Revision 1.3  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.2  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.1  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.29  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.28  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/europeanoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace Pricers {

        double EuropeanOption::value() const {
          if(!hasBeenCalculated_) {
            dividendDiscount_ = (QL_EXP(-dividendYield_*residualTime_));
            riskFreeDiscount_ = (QL_EXP(-riskFreeRate_*residualTime_));
              standardDeviation_ = volatility_*QL_SQRT(residualTime_);
              Math::CumulativeNormalDistribution f;
              double D1 = QL_LOG(underlying_/strike_)/standardDeviation_ +
                    standardDeviation_/2.0 + (riskFreeRate_ - dividendYield_)*
                                            residualTime_/standardDeviation_;
              double D2 = D1 - standardDeviation_;
              double fD1 = f(D1), fD2 = f(D2);
              switch (type_) {
                  case Call:
                  alpha_ = fD1;
                  beta_ = fD2;
                  NID1_ = f.derivative(D1);
                  break;
                  case Put:
                  alpha_ = fD1-1.0;
                  beta_ = fD2-1.0;
                  NID1_ = f.derivative(D1);
                  break;
                  case Straddle:
                  alpha_ = 2.0*fD1-1.0;
                  beta_ = 2.0*fD2-1.0;
                  NID1_ = 2.0*f.derivative(D1);
                  break;
                  default:
                  throw IllegalArgumentError(
                    "EuropeanOption: invalid option type");
            }
            hasBeenCalculated_ = true;
            value_ = underlying_ * dividendDiscount_ * alpha_ -
                                    strike_ * riskFreeDiscount_ * beta_;
          }
            return value_;
        }

        double EuropeanOption::delta() const {
          if(!hasBeenCalculated_)
            value();

            return dividendDiscount_*alpha_;
        }

        double EuropeanOption::gamma() const {
          if(!hasBeenCalculated_)
            value();

            return NID1_*dividendDiscount_/(underlying_*standardDeviation_);
        }

        double EuropeanOption::theta() const {
          if(!hasBeenCalculated_)
            value();

            return -underlying_ * NID1_ * volatility_ *
                dividendDiscount_/(2.0*QL_SQRT(residualTime_)) +
                  dividendYield_*underlying_*alpha_*dividendDiscount_ -
                        riskFreeRate_*strike_*riskFreeDiscount_*beta_;
        }

        double EuropeanOption::rho() const {
          if(!hasBeenCalculated_)
            value();

            return residualTime_*riskFreeDiscount_*strike_*beta_;
        }

        double EuropeanOption::vega() const {
          if(!hasBeenCalculated_)
            value();

            return underlying_*NID1_*dividendDiscount_*QL_SQRT(residualTime_);
        }

    }

}
