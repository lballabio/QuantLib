
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

/*!  \file binaryoption.cpp
    \fullpath Sources/Pricers/%binaryoption.cpp
   \brief European style cash-or-nothing option.

*/

// $Id$
// $Log$
// Revision 1.13  2001/08/13 15:06:17  nando
// added dividendRho method
//
// Revision 1.12  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.11  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.10  2001/08/07 11:25:55  sigmud
// copyright header maintenance
//
// Revision 1.9  2001/08/06 15:43:34  nando
// BSMOption now is SingleAssetOption
// BSMEuropeanOption now is EuropeanOption
//
// Revision 1.8  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Pricers/binaryoption.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib
{
    namespace Pricers
    {
        BinaryOption::BinaryOption(Type type, double underlying, double strike,
                                   Rate dividendYield, Rate
                                   riskFreeRate, Time residualTime,
                                   double volatility, double
                                   cashPayoff)
            : SingleAssetOption(type, underlying, strike, dividendYield,
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

        Handle<SingleAssetOption> BinaryOption::clone() const {
//            QL_REQUIRE(hasBeenInitialized,
            //                     "BinaryOption::clone() : BinaryOption must be initialized");

            return Handle<SingleAssetOption>(new BinaryOption(*this));
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

        double BinaryOption::dividendRho() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");

            if(type_ == Option::Straddle)
                return 0.0;
            else {
                double D2IT = residualTime_/volSqrtTime_;
                return -cashPayoff_*discount_*optionSign_*(D2IT*NID2_);
            }
        }

        double BinaryOption::vega() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");
            return -optionSign_ * cashPayoff_ * discount_ * NID2_ * D1_/volatility_;
        }


    }
}







