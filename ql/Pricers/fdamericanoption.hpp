
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file americanoption.hpp
    \brief american option

    \fullpath
    ql/Pricers/%americanoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_american_option_h
#define quantlib_pricers_american_option_h

#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/FiniteDifferences/americancondition.hpp>

namespace QuantLib {

    namespace Pricers {

        //! American option
        //! \todo make american call with no dividens = european
        class FdAmericanOption : public FdStepConditionOption {
          public:
            // constructor
            FdAmericanOption(Option::Type type, double underlying, double strike,
                           Spread dividendYield, Rate riskFreeRate,
                           Time residualTime, double volatility,
                           int timeSteps, int gridPoints);
                void initializeStepCondition() const;

            // This method must be implemented to imply volatilities
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(new FdAmericanOption(*this));
            }
        };


        // inline definitions

        inline FdAmericanOption::FdAmericanOption(Option::Type type,
            double underlying, double strike, Spread dividendYield,
            Rate riskFreeRate, Time residualTime, double volatility,
            int timeSteps, int gridPoints)
        : FdStepConditionOption(type, underlying, strike, dividendYield,
            riskFreeRate, residualTime, volatility, timeSteps,
            gridPoints) {}

        inline void FdAmericanOption::initializeStepCondition() const {
            stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                new FiniteDifferences::AmericanCondition(initialPrices_));
        }

    }

}


#endif

