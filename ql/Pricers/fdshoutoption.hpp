

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
/*! \file shoutoption.hpp
    \brief shout option

    \fullpath
    ql/Pricers/%shoutoption.hpp
*/

// $Id$

#ifndef quantlib_pricers_shout_option_h
#define quantlib_pricers_shout_option_h

#include <ql/Pricers/fdstepconditionoption.hpp>
#include <ql/FiniteDifferences/shoutcondition.hpp>

namespace QuantLib {

    namespace Pricers {

        class FdShoutOption : public FdStepConditionOption {
          public:
            // constructor
            FdShoutOption(Option::Type type, double underlying, double strike,
                           Spread dividendYield, Rate riskFreeRate,
                           Time residualTime, double volatility,
                           int timeSteps, int gridPoints);
                void initializeStepCondition() const;

            // This method must be implemented to imply volatilities
            Handle<SingleAssetOption> clone() const{
                return Handle<SingleAssetOption>(new FdShoutOption(*this));
            }
        };


        // inline definitions

        inline FdShoutOption::FdShoutOption(Option::Type type, double underlying,
            double strike, Spread dividendYield, Rate riskFreeRate,
            Time residualTime, double volatility, int timeSteps,
            int gridPoints)
        : FdStepConditionOption(type, underlying, strike, dividendYield,
                             riskFreeRate, residualTime, volatility,
                             timeSteps, gridPoints){}

        inline void FdShoutOption::initializeStepCondition() const {
            stepCondition_ = Handle<FiniteDifferences::StandardStepCondition>(
                new FiniteDifferences::ShoutCondition(intrinsicValues_,
                residualTime_, riskFreeRate_));
        }

    }

}


#endif

