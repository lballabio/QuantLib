
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

/*!  \file binaryoption.cpp
   \brief European style cash-or-nothing option.

  $Source$

  $Log$
  Revision 1.4  2001/04/06 18:46:21  nando
  changed Authors, Contributors, Licence and copyright header

  Revision 1.3  2001/04/04 12:13:24  nando
  Headers policy part 2:
  The Include directory is added to the compiler's include search path.
  Then both your code and user code specifies the sub-directory in
  #include directives, as in
  #include <Solvers1d/newton.hpp>

  Revision 1.2  2001/04/04 11:07:24  nando
  Headers policy part 1:
  Headers should have a .hpp (lowercase) filename extension
  All *.h renamed to *.hpp

  Revision 1.1  2001/03/01 12:57:39  enri
  class BinaryOption added, test it with binary_option.py

*/

#include "Pricers/binaryoption.hpp"
#include "Math/normaldistribution.hpp"

namespace QuantLib
{
    namespace Pricers
    {
        BinaryOption::BinaryOption(Type type, double underlying, double strike,
                                   Rate dividendYield, Rate
                                   riskFreeRate, Time residualTime,
                                   double volatility, double
                                   cashPayoff)
            : BSMOption(type, underlying, strike, dividendYield,
                        riskFreeRate, residualTime, volatility),
            cashPayoff_(cashPayoff)
        {
            D1_ = QL_LOG(underlying_/strike_)/volSqrtTime_ +
                volSqrtTime_/2.0 + (riskFreeRate_ - dividendYield_)*
                residualTime_/volSqrtTime_;
            discount_ = QL_EXP(-riskFreeRate_ * residualTime_);

            volSqrtTime_ = volatility_ * QL_SQRT(residualTime_);
            D1_ = QL_LOG(underlying_ / strike_) / volSqrtTime_
                + (riskFreeRate_ - dividendYield_) * residualTime_ / volSqrtTime_
                + volSqrtTime_/2.0;
            D2_ = D1_ - volSqrtTime_;

            Math::CumulativeNormalDistribution f;

            ND2_ = f(D2_);

            switch (type_) {
            case Option::Call:
                optionSign_ = 1.0;
                beta_ = ND2_;
                NID2_ = f.derivative(D2_);
                break;
            case Option::Put:
                optionSign_ = -1.0;
                beta_ = ND2_ - 1.0;
                NID2_ = f.derivative(D2_);
                break;
            case Option::Straddle:
                optionSign_ = 0.0;
                beta_ = 2.0 * ND2_ - 1.0;
                NID2_ = 2.0 * f.derivative(D2_);
                break;
            default:
                throw IllegalArgumentError("AnalyticBSM: invalid option type");

            }
        }

        Handle<BSMOption> BinaryOption::clone() const {
//            QL_REQUIRE(hasBeenInitialized,
            //                     "BinaryOption::clone() : BinaryOption must be initialized");

            return Handle<BSMOption>(new BinaryOption(*this));
        }

        double BinaryOption::value() const {
//            QL_REQUIRE(hasBeenInitialized,
//                       "BinaryOption::value() : BinaryOption must be initialized");
            double inTheMoneyProbability;
            switch (type_) {
            case Option::Call:
                inTheMoneyProbability = ND2_;
                break;
            case Option::Put:
                inTheMoneyProbability = 1.0 - ND2_;
                break;
            case Option::Straddle:
                inTheMoneyProbability = 1.0;
                break;
            default:
                throw IllegalArgumentError("AnalyticBSM: invalid option type");
            }
            return cashPayoff_ * discount_ * inTheMoneyProbability;
        }

        double BinaryOption::delta() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");
            return optionSign_*cashPayoff_*discount_*NID2_/(underlying_*volSqrtTime_);
        }

        double BinaryOption::gamma() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");
            return -cashPayoff_ * discount_ * optionSign_ * NID2_ *
                ( 1.0 + D2_/volSqrtTime_) / (underlying_ *
                                             underlying_ *
                                             volSqrtTime_);
        }

        double BinaryOption::theta() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");

            if(type_ == Option::Straddle) {
                return cashPayoff_*discount_*riskFreeRate_;
            } else {
                double D2IT = (-QL_LOG(underlying_ / strike_) / volSqrtTime_
                               + (riskFreeRate_ - dividendYield_) *
                               residualTime_ / volSqrtTime_
                               - volSqrtTime_ / 2.0)/(2.0 * residualTime_);
                return -cashPayoff_ * discount_ * optionSign_ *
                    ( D2IT * NID2_ - riskFreeRate_ * beta_);
            }
        }

        double BinaryOption::rho() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");

            if(type_ == Option::Straddle)
                return -cashPayoff_*residualTime_*discount_;
            else {
                double D2IT = residualTime_/volSqrtTime_;
                return cashPayoff_*discount_*optionSign_*(D2IT*NID2_-residualTime_*beta_);
            }
        }

        double BinaryOption::vega() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");
            return -optionSign_ * cashPayoff_ * discount_ * NID2_ * D1_/volatility_;
        }


    }
}







