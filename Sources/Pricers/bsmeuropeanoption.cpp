
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

/*! \file bsmeuropeanoption.cpp
    \brief european option

    $Source$
    $Log$
    Revision 1.27  2001/05/24 13:57:52  nando
    smoothing #include xx.hpp and cutting old Log messages

*/

#include "ql/Pricers/bsmeuropeanoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace Pricers {

        double BSMEuropeanOption::value() const {
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
                    "BSMEuropeanOption: invalid option type");
            }
            hasBeenCalculated_ = true;
            value_ = underlying_ * dividendDiscount_ * alpha_ -
                                    strike_ * riskFreeDiscount_ * beta_;
          }
            return value_;
        }

        double BSMEuropeanOption::delta() const {
          if(!hasBeenCalculated_)
            value();

            return dividendDiscount_*alpha_;
        }

        double BSMEuropeanOption::gamma() const {
          if(!hasBeenCalculated_)
            value();

            return NID1_*dividendDiscount_/(underlying_*standardDeviation_);
        }

        double BSMEuropeanOption::theta() const {
          if(!hasBeenCalculated_)
            value();

            return -underlying_ * NID1_ * volatility_ *
                dividendDiscount_/(2.0*QL_SQRT(residualTime_)) +
                  dividendYield_*underlying_*alpha_*dividendDiscount_ -
                        riskFreeRate_*strike_*riskFreeDiscount_*beta_;
        }

        double BSMEuropeanOption::rho() const {
          if(!hasBeenCalculated_)
            value();

            return residualTime_*riskFreeDiscount_*strike_*beta_;
        }

        double BSMEuropeanOption::vega() const {
          if(!hasBeenCalculated_)
            value();

            return underlying_*NID1_*dividendDiscount_*QL_SQRT(residualTime_);
        }

    }

}
