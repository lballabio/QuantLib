
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binaryoptionpricer.cpp
    \brief European style cash-or-nothing option pricer
*/

#include <ql/Pricers/binaryoptionpricer.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib
{
    namespace Pricers
    {
        BinaryOption::BinaryOption(Option::Type type, double underlying,
                                   double strike, Spread dividendYield,
                                   Rate riskFreeRate, Time residualTime,
                                   double volatility, double cashPayoff)
            : SingleAssetOption(type, underlying, strike, dividendYield,
                        riskFreeRate, residualTime, volatility),
            cashPayoff_(cashPayoff)
        {
            discount_ = QL_EXP(-riskFreeRate_ * residualTime_);
            volSqrtTime_ = volatility_ * QL_SQRT(residualTime_);

            D1_ = QL_LOG(underlying_ / payoff_.strike()) / volSqrtTime_
                + (riskFreeRate_ - dividendYield_) * residualTime_ / volSqrtTime_
                + volSqrtTime_/2.0;
            D2_ = D1_ - volSqrtTime_;

            Math::CumulativeNormalDistribution f;

            ND2_ = f(D2_);

            switch (payoff_.optionType()) {
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
            switch (payoff_.optionType()) {
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

            if(payoff_.optionType() == Option::Straddle) {
                return cashPayoff_*discount_*riskFreeRate_;
            } else {
                double D2IT = (-QL_LOG(underlying_ / payoff_.strike()) / volSqrtTime_
                               + (riskFreeRate_ - dividendYield_) *
                               residualTime_ / volSqrtTime_
                               - volSqrtTime_ / 2.0)/(2.0 * residualTime_);
                return -cashPayoff_ * discount_ * optionSign_ *
                    ( D2IT * NID2_ - riskFreeRate_ * beta_);
            }
        }

        double BinaryOption::rho() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");

            if(payoff_.optionType() == Option::Straddle)
                return -cashPayoff_*residualTime_*discount_;
            else {
                double D2IT = residualTime_/volSqrtTime_;
                return cashPayoff_*discount_*optionSign_*(D2IT*NID2_-residualTime_*beta_);
            }
        }

        double BinaryOption::dividendRho() const {
//            QL_REQUIRE(hasBeenInitialized, "BinaryOption must be initialized");

            if(payoff_.optionType() == Option::Straddle)
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







