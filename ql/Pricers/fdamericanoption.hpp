
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

