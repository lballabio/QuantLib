
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdamericanoption.hpp
    \brief american option
*/

#ifndef quantlib_pricers_american_option_h
#define quantlib_pricers_american_option_h

#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/FiniteDifferences/americancondition.hpp>

namespace QuantLib {

    //! American option
    class FdAmericanOption : public FdStepConditionOption {
      public:
        // constructor
        FdAmericanOption(Option::Type type,
                         Real underlying,
                         Real strike,
                         Spread dividendYield,
                         Rate riskFreeRate,
                         Time residualTime,
                         Volatility volatility,
                         Size timeSteps = 100,
                         Size gridPoints = 100);
        void initializeStepCondition() const;

        // This method must be implemented to imply volatilities
        boost::shared_ptr<SingleAssetOption> clone() const{
            return boost::shared_ptr<SingleAssetOption>(
                                                 new FdAmericanOption(*this));
        }
    };


    // inline definitions

    inline FdAmericanOption::FdAmericanOption(
                         Option::Type type, Real underlying, Real strike,
                         Spread dividendYield, Rate riskFreeRate,
                         Time residualTime, Volatility volatility,
                         Size timeSteps, Size gridPoints)
    : FdStepConditionOption(type, underlying, strike, dividendYield,
                            riskFreeRate, residualTime, volatility, timeSteps,
                            gridPoints) {}

    inline void FdAmericanOption::initializeStepCondition() const {
        stepCondition_ = boost::shared_ptr<StandardStepCondition>(
                                     new AmericanCondition(intrinsicValues_));
    }

}


#endif

